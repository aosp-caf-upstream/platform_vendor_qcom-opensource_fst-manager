/*
 * FST Manager: FST Configuration Manager
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

#ifndef __FST_FSTCFGMGR_H__
#define __FST_FSTCFGMGR_H__
#include "utils/common.h"
#include "fst_ctrl.h"

enum fst_config_method {
	FST_CONFIG_CLI,
	FST_CONFIG_INI
};


int  fst_cfgmgr_init(enum fst_config_method m, void *ctx);
void fst_cfgmgr_deinit();

int fst_cfgmgr_get_group_ifaces(const struct fst_group_info *group,
	struct fst_iface_info **ifaces);
int fst_cfgmgr_get_groups(struct fst_group_info **groups);

int fst_cfgmgr_on_global_init(void);
void fst_cfgmgr_on_global_deinit(void);
int fst_cfgmgr_on_group_init(const struct fst_group_info *group);
int fst_cfgmgr_on_group_deinit(const struct fst_group_info *group);
int fst_cfgmgr_on_iface_init(const struct fst_group_info *group,
	struct fst_iface_info *iface);
int fst_cfgmgr_on_iface_deinit(struct fst_iface_info *iface);
int fst_cfgmgr_on_connect(struct fst_group_info *group, const char *iface,
	const u8* addr);
int fst_cfgmgr_on_disconnect(struct fst_group_info *group, const char *iface,
	const u8* addr);
int fst_cfgmgr_get_mux_type(const char *gname, char *buf, int blen);
int fst_cfgmgr_get_mux_ifname(const char *gname, char *buf, int blen);
Boolean fst_cfgmgr_is_mux_managed(const char *gname);

#endif /* __FST_FSTCONF_H */

