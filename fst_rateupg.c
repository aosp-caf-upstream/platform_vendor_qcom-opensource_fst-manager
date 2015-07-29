/*
 * FST Manager: Rate Upgrade
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "utils/list.h"
#include "fst_rateupg.h"

#define FST_MGR_COMPONENT "RATEUPG"
#include "fst_manager.h"

struct rate_upgrade_group {
	char                  *groupname;
	char                  *master;
	struct fst_iface_info *slaves;
	int                         slave_cnt;
	struct dl_list              lentry;
};

struct rate_upgrade_manager {
	struct fst_ini_config *iniconf;
	struct dl_list        groups;
};

static struct rate_upgrade_manager g_rateupg_mgr;
static int            g_rateupg_mgr_initialized = 0;

static struct rate_upgrade_group *find_rate_upgrade_group(const char *name)
{
	struct rate_upgrade_group *g;
	dl_list_for_each(g, &g_rateupg_mgr.groups, struct rate_upgrade_group, lentry) {
		if(!strncmp(g->groupname, name, strlen(name)))
			return g;
	}
	return NULL;
}

static void deinit_rate_upgrade_group(struct rate_upgrade_group *g)
{
	free(g->slaves);
	free(g->master);
	free(g->groupname);
	dl_list_del(&g->lentry);
	free(g);
}

int fst_rate_upgrade_init(struct fst_ini_config *h)
{
	os_memset(&g_rateupg_mgr, 0, sizeof(g_rateupg_mgr));
	dl_list_init(&g_rateupg_mgr.groups);
	g_rateupg_mgr_initialized = 1;
	g_rateupg_mgr.iniconf = h;
	return 0;
}

void fst_rate_upgrade_deinit()
{
	if (g_rateupg_mgr_initialized) {
		while (!dl_list_empty(&g_rateupg_mgr.groups)) {
			struct rate_upgrade_group *g = dl_list_first(&g_rateupg_mgr.groups,
					struct rate_upgrade_group, lentry);
			deinit_rate_upgrade_group(g);
		}
		g_rateupg_mgr_initialized = 0;
	}
}

int fst_rate_upgrade_add_group(const struct fst_group_info *group)
{
	struct rate_upgrade_group *g;
	struct fst_iface_info *ifaces;
	int i;

	if (find_rate_upgrade_group(group->id)) {
		fst_mgr_printf(MSG_WARNING, "Group %s already added", group->id);
		return 0;
	}
	char *master = fst_ini_config_get_rate_upgrade_master(
		g_rateupg_mgr.iniconf, group->id);
	if (master) {
		g = malloc(sizeof(struct rate_upgrade_group));
		memset(g, 0, sizeof(struct rate_upgrade_group));
		g->master = master;
		g->groupname = strdup(group->id);
		if (g->groupname == NULL) {
			fst_mgr_printf(MSG_ERROR, "Cannot alloc groupname %s",
				group->id);
			goto error_groupname;
		}
		g->slave_cnt = fst_ini_config_get_group_slave_ifaces(
			g_rateupg_mgr.iniconf, group, master, &ifaces);
		if (g->slave_cnt < 0) {
			fst_mgr_printf(MSG_ERROR, "Cannot add group %s", group->id);
			goto error_get_slave;
		}
		else if (g->slave_cnt == 0) {
			fst_mgr_printf(MSG_ERROR,
				"No slave ifaces found in group %s", group->id);
			goto error_get_slave;
		}
		g->slaves = ifaces;

		for (i = 0; i < g->slave_cnt; i++) {
			if (fst_add_iface(master, &ifaces[i])) {
				fst_mgr_printf(MSG_ERROR,
				"Cannot add slave interface %s", ifaces[i].name);
				goto error_add;
			}
		}
		dl_list_add_tail(&g_rateupg_mgr.groups, &g->lentry);
	}
	return 0;
error_add:
	while(i-- > 0)
		fst_del_iface(&ifaces[i]);
	free(ifaces);
error_get_slave:
	free(g->groupname);
error_groupname:
	free(master);
	free(g);
	return -1;
}

int fst_rate_upgrade_del_group(const struct fst_group_info *group)
{
	struct rate_upgrade_group *g;
	int i;

	g = find_rate_upgrade_group(group->id);
	if (g == NULL) {
		fst_mgr_printf(MSG_ERROR, "No group exists %s", group->id);
		return -1;
	}

	for (i = 0; i < g->slave_cnt; i++) {
		if (fst_del_iface(&g->slaves[i])) {
			fst_mgr_printf(MSG_ERROR, "Cannot delete iface %s",
				g->slaves[i].name);
		}
	}

	deinit_rate_upgrade_group(g);
	return 0;
}


int fst_rate_upgrade_on_connect(const struct fst_group_info *group,
	const char *iface, const u8* addr)
{
	int i;
	struct rate_upgrade_group *g;

	g = find_rate_upgrade_group(group->id);
	if (g && strncmp(iface, g->master, strlen(iface)) == 0) {
		for (i = 0; i < g->slave_cnt; i++) {
			if (fst_dup_connection(&g->slaves[i],
			   g->master, addr)) {
				fst_mgr_printf(MSG_ERROR, "Cannot connect iface %s",
				g->slaves[i].name);
				goto error_connect;
			}
		}
	}
	return 0;

error_connect:
	while(i-- > 0) {
		fst_dedup_connection(&g->slaves[i]);
	}
	return -1;
}

int fst_rate_upgrade_on_disconnect(const struct fst_group_info *group,
	const char *iface, const u8* addr)
{
	int i, res = 0;
	struct rate_upgrade_group *g;
	g = find_rate_upgrade_group(group->id);
	if (g && strncmp(iface, g->master, strlen(iface)) == 0) {
		for (i = 0; i < g->slave_cnt; i++) {
			if (fst_dedup_connection(&g->slaves[i])) {
				fst_mgr_printf(MSG_ERROR, "Cannot disconnect iface %s",
				g->slaves[i].name);
				res = -1;
			}
		}
	}
	return res;
}
