#pragma once

#include <utils/common.h>

typedef struct {
    uint16_t max_power;
    uint8_t  rsvd2;
    uint8_t  flags;
    uint32_t entry_lat;
    uint32_t exit_lat;
    uint8_t  read_tput;
    uint8_t  read_lat;
    uint8_t  write_tput;
    uint8_t  write_lat;
    uint16_t idle_power;
    uint8_t  idle_scale;
    uint8_t  rsvd19;
    uint16_t active_power;
    uint8_t  active_work_scale;
    uint8_t  rsvd23[9];
} __attribute__((packed)) nvme_id_power_state_t;

typedef struct {
    uint16_t vid;
    uint16_t ssvid;
    char     sn[20];
    char     mn[40];
    char     fr[8];
    uint8_t  rab;
    uint8_t  ieee[3];
    uint8_t  mic;
    uint8_t  mdts;
    uint16_t cntlid;
    uint32_t ver;
    uint8_t  rsvd84[172];
    uint16_t oacs;
    uint8_t  acl;
    uint8_t  aerl;
    uint8_t  frmw;
    uint8_t  lpa;
    uint8_t  elpe;
    uint8_t  npss;
    uint8_t  avscc;
    uint8_t  apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint8_t  rsvd270[242];
    uint8_t  sqes;
    uint8_t  cqes;
    uint8_t  rsvd514[2];
    uint32_t nn;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t  fna;
    uint8_t  vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t  nvscc;
    uint8_t  rsvd531;
    uint16_t acwu;
    uint8_t  rsvd534[2];
    uint32_t sgls;
    uint8_t  rsvd540[1508];
    nvme_id_power_state_t psd[32];
    uint8_t  vs[1024];
} __attribute__((packed)) nvme_id_ctrl_t;

typedef struct {
    uint16_t ms;
    uint8_t ds;
    uint8_t rp;
} __attribute__((packed)) nvme_lbaf_t;

typedef struct {
    uint64_t nsize;
    uint64_t ncap;
    uint64_t nuse;
    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;
    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;
    uint8_t fpi;
    uint8_t dlfeat;
    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;
    uint16_t noiob;
    uint64_t nvmcap[2];
    uint16_t npwg;
    uint16_t npwa;
    uint16_t npdg;
    uint16_t npda;
    uint16_t nows;
    uint8_t reserved1[18];
    uint32_t anagrpid;
    uint8_t reserved2[3];
    uint8_t nsattr;
    uint16_t nvmsetid;
    uint16_t endgid;
    uint8_t nguid[16];
    uint8_t eui64[8];
    nvme_lbaf_t lbaf[16];
    uint8_t rsvd192[192];
    uint8_t vs[3712];
} __attribute__((packed)) nvme_id_ns_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint32_t cdw2[2];
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10[6];
} __attribute__((packed)) nvme_common_command_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t reserved1[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cns;
    uint32_t reserved2[5];
} __attribute__((packed)) nvme_identify_command_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t reserved1[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t fid;
    uint32_t dword11;
    uint32_t reserved2[4];
} __attribute__((packed)) nvme_features_command_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t reserved1[5];
    uint64_t prp1;
    uint64_t reserved2;
    uint16_t cqid;
    uint16_t qsize;
    uint16_t cq_flags;
    uint16_t irq_vector;
    uint32_t reserved3[4];
} __attribute__((packed)) nvme_create_cq_command_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t reserved1[5];
    uint64_t prp1;
    uint64_t reserved2;
    uint16_t sqid;
    uint16_t qsize;
    uint16_t sq_flags;
    uint16_t cqid;
    uint32_t reserved3[4];
} __attribute__((packed)) nvme_create_sq_command_t;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint32_t cdw2[2];
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint64_t startLBA;
    uint16_t length;
    uint16_t control;
    uint32_t dsmgm;
    uint32_t eilbrt;
    uint16_t elbat;
    uint16_t elbatm;
} __attribute__((packed)) nvme_rw_command_t;

typedef struct {
    union {
        nvme_common_command_t common;
        nvme_identify_command_t identify;
        nvme_features_command_t features;
        nvme_create_cq_command_t create_cq;
        nvme_create_sq_command_t create_sq;
        nvme_rw_command_t rw;
    };
} nvme_command_t;

typedef struct {
    uint32_t result;
    uint32_t reserved;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status;
} nvme_result_t;

void nvme_init();