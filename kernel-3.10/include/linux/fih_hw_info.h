 /*
*Copyright(C) 2014-2015 Foxconn International Holdings, Ltd. All rights reserved
*/
#ifndef _FIH_HW_INFO_H
#define _FIH_HW_INFO_H

typedef struct
{
  unsigned int ID;
  char* STR;
  int LEN; //string lengh
}ID2STR_MAP;

typedef enum
{
  PROJECT_HOLLY =0x01,
  PROJECT_END = 0xFE,
  PROJECT_MAX = 0xFF,
} fih_product_id_type;

//notice that the phase must be ordered
typedef enum
{
  PHASE_EVM  = 0x00,
  PHASE_EVM2 = 0x01,
  PHASE_EVM3 = 0x02,
  PHASE_PD  = 0x10,
  PHASE_PD2  = 0x11,
  PHASE_PD3  = 0x12,
  PHASE_PD4  = 0x13,
  PHASE_DP   = 0x20,
  PHASE_Pre_SP=0x29,
  PHASE_SP   = 0x30,
  PHASE_SP5   = 0x35,
  PHASE_SP_AP1   = 0x38,
  PHASE_AP   = 0x40,
  PHASE_AP2   = 0x41,
  PHASE_AP2_AP3   = 0x42,
  PHASE_TP   = 0x50,
  PHASE_TP1_TP2   = 0x54,
  PHASE_PQ   = 0x60,
  PHASE_MP   = 0x70,
  PHASE_END  = 0xFE,
  PHASE_MAX  = 0XFF,
}fih_product_phase_type;

typedef enum
{
  BAND_GINA = 0x01,
  BAND_REX  = 0x02,
  BAND_APAC = 0x03,
  BAND_SAMBA = 0x04,
  BAND_GINA2 = 0x05,
  BAND_REX2  = 0x06,
  BAND_APAC2 = 0x07,
  BAND_SAMBA2 = 0x08,
  BAND_END  = 0xFE,
  BAND_MAX  = 0xFF,
}fih_band_id_type;

typedef enum
{
  SIM_SINGLE =0x01,
  SIM_DUAL   =0x02,
  SIM_END    =0xFE,
  SIM_MAX    =0xFF,
}fih_sim_type;

//============================

static const ID2STR_MAP project_id_map[] =
{
  {PROJECT_HOLLY, "Holly", 5},
  {PROJECT_MAX,   "unknown", 7}
};

static const ID2STR_MAP phase_id_map[] =
{
  {PHASE_EVM,    "EVM",     3},
  {PHASE_EVM2,   "EVM2",    4},
  {PHASE_EVM3,   "EVM3",    4},
  {PHASE_PD,    "PD",     2},
  {PHASE_PD2,    "PD2",     3},
  {PHASE_PD3,    "PD3",     3},
  {PHASE_PD4,    "PD4",     3},
  {PHASE_DP,     "DP",      2},
  {PHASE_Pre_SP, "Pre-SP",  6},
  {PHASE_SP,     "SP",      2},
  {PHASE_SP5,     "SP5",      3},
  {PHASE_SP_AP1,     "SP_AP1",      6},
  {PHASE_AP,     "AP",      2},
  {PHASE_AP2,     "AP2",      3},
  {PHASE_AP2_AP3,     "AP2_AP3",      7},
  {PHASE_PQ,     "PQ",      2},
  {PHASE_TP,     "TP",      2},
  {PHASE_TP1_TP2,     "TP1_TP2",      7},
  {PHASE_MP,     "MP",      2},
  {PHASE_END,    "END",     3},
  {PHASE_MAX,    "unknown", 7}
};

static const ID2STR_MAP band_id_map[] =
{
  {BAND_GINA, "GINA", 4},
  {BAND_REX,  "REX",  3},
  {BAND_APAC, "APAC", 4},
  {BAND_SAMBA, "SAMBA", 5},
  {BAND_GINA2, "GINA2", 5},
  {BAND_REX2,  "REX2",  4},
  {BAND_APAC2, "APAC2", 5},
  {BAND_SAMBA2, "SAMBA2", 6},
  {BAND_MAX,  "unknown", 7}
};

static const ID2STR_MAP sim_id_map[] =
{
  {SIM_SINGLE, "SINGLE",  6},
  {SIM_DUAL,   "DUAL",    4},
  {SIM_MAX,    "unknown", 7}
};

//============================
//knox void fih_set_oem_info(void);
void fih_proc_init(void);

//knox unsigned int fih_get_s1_boot(void);
unsigned int fih_get_product_id(void);
unsigned int fih_get_product_phase(void);
unsigned int fih_get_band_id(void);
unsigned int fih_get_sim_id(void);

int fih_phase_match(fih_product_phase_type start,fih_product_phase_type end);
//knox int fih_dts_phase_match(const char* start, const char* end);
//knox void fih_dts_phase_cfg(fih_product_phase_type start,fih_product_phase_type end, char* str);
void fih_hwid_get(void);
//knox char *fih_get_nonHLOS_version(void);
//knox char *fih_get_nonHLOS_git_head(void);
//knox void fih_get_nonHLOS_info(void);

#endif
