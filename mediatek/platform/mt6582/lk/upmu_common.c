#include <platform/mt_pmic.h>
#include <printf.h>

void pmic_lock(void){    
}

void pmic_unlock(void){    
}

void upmu_set_rg_vcdt_hv_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCDT_HV_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCDT_HV_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_chr_ldo_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON0),
                           (&val),
                           (kal_uint32)(PMIC_RGS_CHR_LDO_DET_MASK),
                           (kal_uint32)(PMIC_RGS_CHR_LDO_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_pchr_automode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_AUTOMODE_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_AUTOMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_EN_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chr_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHR_EN_MASK),
                             (kal_uint32)(PMIC_RG_CHR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_chrdet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON0),
                           (&val),
                           (kal_uint32)(PMIC_RGS_CHRDET_MASK),
                           (kal_uint32)(PMIC_RGS_CHRDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rgs_vcdt_lv_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON0),
                           (&val),
                           (kal_uint32)(PMIC_RGS_VCDT_LV_DET_MASK),
                           (kal_uint32)(PMIC_RGS_VCDT_LV_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rgs_vcdt_hv_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON0),
                           (&val),
                           (kal_uint32)(PMIC_RGS_VCDT_HV_DET_MASK),
                           (kal_uint32)(PMIC_RGS_VCDT_HV_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcdt_lv_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCDT_LV_VTH_MASK),
                             (kal_uint32)(PMIC_RG_VCDT_LV_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcdt_hv_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCDT_HV_VTH_MASK),
                             (kal_uint32)(PMIC_RG_VCDT_HV_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_cv_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_CV_EN_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_CV_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_cc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_CC_EN_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_CC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_cs_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CS_EN_MASK),
                             (kal_uint32)(PMIC_RG_CS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_cs_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON2),
                           (&val),
                           (kal_uint32)(PMIC_RGS_CS_DET_MASK),
                           (kal_uint32)(PMIC_RGS_CS_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rgs_vbat_cv_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON2),
                           (&val),
                           (kal_uint32)(PMIC_RGS_VBAT_CV_DET_MASK),
                           (kal_uint32)(PMIC_RGS_VBAT_CV_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rgs_vbat_cc_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON2),
                           (&val),
                           (kal_uint32)(PMIC_RGS_VBAT_CC_DET_MASK),
                           (kal_uint32)(PMIC_RGS_VBAT_CC_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vbat_cv_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_CV_VTH_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_CV_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_cc_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_CC_VTH_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_CC_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_cs_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CS_VTH_MASK),
                             (kal_uint32)(PMIC_RG_CS_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_tohtc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_TOHTC_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_TOHTC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_toltc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_TOLTC_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_TOLTC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_ov_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_OV_EN_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_OV_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_ov_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_OV_VTH_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_OV_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbat_ov_deg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBAT_OV_DEG_MASK),
                             (kal_uint32)(PMIC_RG_VBAT_OV_DEG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_vbat_ov_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON6),
                           (&val),
                           (kal_uint32)(PMIC_RGS_VBAT_OV_DET_MASK),
                           (kal_uint32)(PMIC_RGS_VBAT_OV_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_baton_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BATON_EN_MASK),
                             (kal_uint32)(PMIC_RG_BATON_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_baton_ht_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BATON_HT_EN_MASK),
                             (kal_uint32)(PMIC_RG_BATON_HT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_baton_tdet_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BATON_TDET_EN_MASK),
                             (kal_uint32)(PMIC_BATON_TDET_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_baton_ht_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BATON_HT_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_BATON_HT_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_baton_ht_trim_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BATON_HT_TRIM_SET_MASK),
                             (kal_uint32)(PMIC_RG_BATON_HT_TRIM_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_baton_undet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON7),
                           (&val),
                           (kal_uint32)(PMIC_RGS_BATON_UNDET_MASK),
                           (kal_uint32)(PMIC_RGS_BATON_UNDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_csdac_data(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_DATA_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_DATA_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_frc_csvth_usbdl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FRC_CSVTH_USBDL_MASK),
                             (kal_uint32)(PMIC_RG_FRC_CSVTH_USBDL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_pchr_flag_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON10),
                           (&val),
                           (kal_uint32)(PMIC_RGS_PCHR_FLAG_OUT_MASK),
                           (kal_uint32)(PMIC_RGS_PCHR_FLAG_OUT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_pchr_flag_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_FLAG_EN_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_FLAG_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_otg_bvalid_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OTG_BVALID_EN_MASK),
                             (kal_uint32)(PMIC_RG_OTG_BVALID_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_otg_bvalid_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON10),
                           (&val),
                           (kal_uint32)(PMIC_RGS_OTG_BVALID_DET_MASK),
                           (kal_uint32)(PMIC_RGS_OTG_BVALID_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_pchr_flag_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_FLAG_SEL_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_FLAG_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_testmode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_TESTMODE_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_TESTMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_testmode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_TESTMODE_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_TESTMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_RST_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_ft_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_FT_CTRL_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_FT_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrwdt_td(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRWDT_TD_MASK),
                             (kal_uint32)(PMIC_RG_CHRWDT_TD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrwdt_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRWDT_EN_MASK),
                             (kal_uint32)(PMIC_RG_CHRWDT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrwdt_wr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRWDT_WR_MASK),
                             (kal_uint32)(PMIC_RG_CHRWDT_WR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_rv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_RV_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_RV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrwdt_int_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRWDT_INT_EN_MASK),
                             (kal_uint32)(PMIC_RG_CHRWDT_INT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrwdt_flag_wr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRWDT_FLAG_WR_MASK),
                             (kal_uint32)(PMIC_RG_CHRWDT_FLAG_WR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_chrwdt_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON15),
                           (&val),
                           (kal_uint32)(PMIC_RGS_CHRWDT_OUT_MASK),
                           (kal_uint32)(PMIC_RGS_CHRWDT_OUT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_uvlo_vthl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_UVLO_VTHL_MASK),
                             (kal_uint32)(PMIC_RG_UVLO_VTHL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_usbdl_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_USBDL_RST_MASK),
                             (kal_uint32)(PMIC_RG_USBDL_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_usbdl_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_USBDL_SET_MASK),
                             (kal_uint32)(PMIC_RG_USBDL_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_adcin_vsen_mux_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ADCIN_VSEN_MUX_EN_MASK),
                             (kal_uint32)(PMIC_ADCIN_VSEN_MUX_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adcin_vsen_ext_baton_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_EXT_BATON_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_EXT_BATON_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_adcin_vbat_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ADCIN_VBAT_EN_MASK),
                             (kal_uint32)(PMIC_ADCIN_VBAT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_adcin_vsen_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ADCIN_VSEN_EN_MASK),
                             (kal_uint32)(PMIC_ADCIN_VSEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_adcin_vchr_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ADCIN_VCHR_EN_MASK),
                             (kal_uint32)(PMIC_ADCIN_VCHR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_rsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_RSEL_MASK),
                             (kal_uint32)(PMIC_RG_BGR_RSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_unchop_ph(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_UNCHOP_PH_MASK),
                             (kal_uint32)(PMIC_RG_BGR_UNCHOP_PH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_unchop(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_UNCHOP_MASK),
                             (kal_uint32)(PMIC_RG_BGR_UNCHOP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_bb_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_BB_CTRL_MASK),
                             (kal_uint32)(PMIC_RG_BC11_BB_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_RST_MASK),
                             (kal_uint32)(PMIC_RG_BC11_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_vsrc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_VSRC_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_VSRC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rgs_bc11_cmp_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON18),
                           (&val),
                           (kal_uint32)(PMIC_RGS_BC11_CMP_OUT_MASK),
                           (kal_uint32)(PMIC_RGS_BC11_CMP_OUT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_bc11_vref_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_VREF_VTH_MASK),
                             (kal_uint32)(PMIC_RG_BC11_VREF_VTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_cmp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_CMP_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_CMP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_ipd_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_IPD_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_IPD_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_ipu_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_IPU_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_IPU_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bc11_bias_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_BIAS_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_BIAS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_stp_inc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_INC_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_INC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_stp_dec(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_DEC_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_DEC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_dly(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_DLY_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_DLY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_stp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_STP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_low_ich_db(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON22),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LOW_ICH_DB_MASK),
                             (kal_uint32)(PMIC_RG_LOW_ICH_DB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrind_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON22),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRIND_ON_MASK),
                             (kal_uint32)(PMIC_RG_CHRIND_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chrind_dimming(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON22),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHRIND_DIMMING_MASK),
                             (kal_uint32)(PMIC_RG_CHRIND_DIMMING_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_cv_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CV_MODE_MASK),
                             (kal_uint32)(PMIC_RG_CV_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcdt_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCDT_MODE_MASK),
                             (kal_uint32)(PMIC_RG_VCDT_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_csdac_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CSDAC_MODE_MASK),
                             (kal_uint32)(PMIC_RG_CSDAC_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_tracking_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TRACKING_EN_MASK),
                             (kal_uint32)(PMIC_RG_TRACKING_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_hwcv_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_HWCV_EN_MASK),
                             (kal_uint32)(PMIC_RG_HWCV_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ulc_det_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ULC_DET_EN_MASK),
                             (kal_uint32)(PMIC_RG_ULC_DET_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_trim_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TRIM_EN_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TRIM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ichrg_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ICHRG_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_ICHRG_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON25),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ovp_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OVP_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_OVP_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chr_osc_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHR_OSC_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_CHR_OSC_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_qi_bgr_ext_buf_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_QI_BGR_EXT_BUF_EN_MASK),
                             (kal_uint32)(PMIC_QI_BGR_EXT_BUF_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_test_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TEST_EN_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TEST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_test_rstb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TEST_RSTB_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TEST_RSTB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_dac_usbdl_max(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DAC_USBDL_MAX_MASK),
                             (kal_uint32)(PMIC_RG_DAC_USBDL_MAX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_RSV_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_thr_det_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_THR_DET_DIS_MASK),
                             (kal_uint32)(PMIC_THR_DET_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_thr_tmode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_THR_TMODE_MASK),
                             (kal_uint32)(PMIC_RG_THR_TMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_thr_temp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_THR_TEMP_SEL_MASK),
                             (kal_uint32)(PMIC_RG_THR_TEMP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_thr_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_THR_SEL_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_THR_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_thr_hwpdn_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_THR_HWPDN_EN_MASK),
                             (kal_uint32)(PMIC_THR_HWPDN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_thrdet_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_THRDET_SEL_MASK),
                             (kal_uint32)(PMIC_RG_THRDET_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_iref_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_IREF_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_IREF_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_usbdl_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_USBDL_EN_MASK),
                             (kal_uint32)(PMIC_RG_USBDL_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fchr_keydet_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FCHR_KEYDET_EN_MASK),
                             (kal_uint32)(PMIC_RG_FCHR_KEYDET_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fchr_pu_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FCHR_PU_EN_MASK),
                             (kal_uint32)(PMIC_RG_FCHR_PU_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_en_drvsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EN_DRVSEL_MASK),
                             (kal_uint32)(PMIC_RG_EN_DRVSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rst_drvsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RST_DRVSEL_MASK),
                             (kal_uint32)(PMIC_RG_RST_DRVSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vref_bg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VREF_BG_MASK),
                             (kal_uint32)(PMIC_RG_VREF_BG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pmu_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PMU_RSV_MASK),
                             (kal_uint32)(PMIC_RG_PMU_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_thr_test(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_THR_TEST_MASK),
                             (kal_uint32)(PMIC_THR_TEST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_pmu_thr_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(STRUP_CON5),
                           (&val),
                           (kal_uint32)(PMIC_PMU_THR_DEB_MASK),
                           (kal_uint32)(PMIC_PMU_THR_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_pmu_thr_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(STRUP_CON5),
                           (&val),
                           (kal_uint32)(PMIC_PMU_THR_STATUS_MASK),
                           (kal_uint32)(PMIC_PMU_THR_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_dduvlo_deb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DDUVLO_DEB_EN_MASK),
                             (kal_uint32)(PMIC_DDUVLO_DEB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_pwrbb_deb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_PWRBB_DEB_EN_MASK),
                             (kal_uint32)(PMIC_PWRBB_DEB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_osc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_OSC_EN_MASK),
                             (kal_uint32)(PMIC_STRUP_OSC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_osc_en_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_OSC_EN_SEL_MASK),
                             (kal_uint32)(PMIC_STRUP_OSC_EN_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_ft_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_FT_CTRL_MASK),
                             (kal_uint32)(PMIC_STRUP_FT_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_pwron_force(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_PWRON_FORCE_MASK),
                             (kal_uint32)(PMIC_STRUP_PWRON_FORCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_bias_gen_en_force(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_FORCE_MASK),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_FORCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_pwron(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_PWRON_MASK),
                             (kal_uint32)(PMIC_STRUP_PWRON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_pwron_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_PWRON_SEL_MASK),
                             (kal_uint32)(PMIC_STRUP_PWRON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_bias_gen_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_MASK),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_bias_gen_en_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_SEL_MASK),
                             (kal_uint32)(PMIC_BIAS_GEN_EN_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rtc_xosc32_enb_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RTC_XOSC32_ENB_SW_MASK),
                             (kal_uint32)(PMIC_RTC_XOSC32_ENB_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rtc_xosc32_enb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RTC_XOSC32_ENB_SEL_MASK),
                             (kal_uint32)(PMIC_RTC_XOSC32_ENB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_dig_io_pg_force(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_DIG_IO_PG_FORCE_MASK),
                             (kal_uint32)(PMIC_STRUP_DIG_IO_PG_FORCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VPROC_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VSYS_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vm_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VM_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VM_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio18_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO18_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VIO18_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vtcxo_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VTCXO_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VTCXO_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_va_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VA_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VA_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio28_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO28_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VIO28_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp2_pg_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP2_PG_ENB_MASK),
                             (kal_uint32)(PMIC_VGP2_PG_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_pg_h2l_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_PG_H2L_EN_MASK),
                             (kal_uint32)(PMIC_VPROC_PG_H2L_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_pg_h2l_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_PG_H2L_EN_MASK),
                             (kal_uint32)(PMIC_VSYS_PG_H2L_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_con6_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_CON6_RSV0_MASK),
                             (kal_uint32)(PMIC_STRUP_CON6_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_clr_just_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_CLR_JUST_RST_MASK),
                             (kal_uint32)(PMIC_CLR_JUST_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_uvlo_l2h_deb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_UVLO_L2H_DEB_EN_MASK),
                             (kal_uint32)(PMIC_UVLO_L2H_DEB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_just_pwrkey_rst(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(STRUP_CON8),
                           (&val),
                           (kal_uint32)(PMIC_JUST_PWRKEY_RST_MASK),
                           (kal_uint32)(PMIC_JUST_PWRKEY_RST_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_osc_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(STRUP_CON8),
                           (&val),
                           (kal_uint32)(PMIC_QI_OSC_EN_MASK),
                           (kal_uint32)(PMIC_QI_OSC_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_strup_ext_pmic_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_EXT_PMIC_EN_MASK),
                             (kal_uint32)(PMIC_STRUP_EXT_PMIC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_ext_pmic_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_EXT_PMIC_SEL_MASK),
                             (kal_uint32)(PMIC_STRUP_EXT_PMIC_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_con8_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_CON8_RSV0_MASK),
                             (kal_uint32)(PMIC_STRUP_CON8_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_ext_pmic_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(STRUP_CON9),
                           (&val),
                           (kal_uint32)(PMIC_QI_EXT_PMIC_EN_MASK),
                           (kal_uint32)(PMIC_QI_EXT_PMIC_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_strup_auxadc_start_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_AUXADC_START_SW_MASK),
                             (kal_uint32)(PMIC_STRUP_AUXADC_START_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_auxadc_rstb_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_AUXADC_RSTB_SW_MASK),
                             (kal_uint32)(PMIC_STRUP_AUXADC_RSTB_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_auxadc_start_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_AUXADC_START_SEL_MASK),
                             (kal_uint32)(PMIC_STRUP_AUXADC_START_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_auxadc_rstb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_AUXADC_RSTB_SEL_MASK),
                             (kal_uint32)(PMIC_STRUP_AUXADC_RSTB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_pwroff_seq_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_PWROFF_SEQ_EN_MASK),
                             (kal_uint32)(PMIC_STRUP_PWROFF_SEQ_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_strup_pwroff_preoff_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(STRUP_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STRUP_PWROFF_PREOFF_EN_MASK),
                             (kal_uint32)(PMIC_STRUP_PWROFF_PREOFF_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_EN_L_MASK),
                             (kal_uint32)(PMIC_SPK_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spkmode_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPKMODE_L_MASK),
                             (kal_uint32)(PMIC_SPKMODE_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_EN_L_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_oc_shdn_dl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OC_SHDN_DL_MASK),
                             (kal_uint32)(PMIC_SPK_OC_SHDN_DL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_ther_shdn_l_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_THER_SHDN_L_EN_MASK),
                             (kal_uint32)(PMIC_SPK_THER_SHDN_L_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_gainl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_GAINL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_GAINL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_da_spk_offset_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_DA_SPK_OFFSET_L_MASK),
                           (kal_uint32)(PMIC_DA_SPK_OFFSET_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_da_spk_lead_dglh_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_DA_SPK_LEAD_DGLH_L_MASK),
                           (kal_uint32)(PMIC_DA_SPK_LEAD_DGLH_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_spk_lead_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_NI_SPK_LEAD_L_MASK),
                           (kal_uint32)(PMIC_NI_SPK_LEAD_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_spk_offset_l_ov(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_SPK_OFFSET_L_OV_MASK),
                           (kal_uint32)(PMIC_SPK_OFFSET_L_OV_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_spk_offset_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OFFSET_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_OFFSET_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_lead_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_LEAD_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_LEAD_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_offset_l_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OFFSET_L_MODE_MASK),
                             (kal_uint32)(PMIC_SPK_OFFSET_L_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_spk_trim_done_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_SPK_TRIM_DONE_L_MASK),
                           (kal_uint32)(PMIC_SPK_TRIM_DONE_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_spk_intg_rst_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_INTG_RST_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_INTG_RST_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_force_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_FORCE_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_FORCE_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_slew_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_SLEW_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_SLEW_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spkab_obias_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPKAB_OBIAS_L_MASK),
                             (kal_uint32)(PMIC_RG_SPKAB_OBIAS_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spkrcv_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPKRCV_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPKRCV_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_drc_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_DRC_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_DRC_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_test_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_TEST_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_TEST_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spkab_oc_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPKAB_OC_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPKAB_OC_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_oc_en_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_OC_EN_L_MASK),
                             (kal_uint32)(PMIC_RG_SPK_OC_EN_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_wnd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_WND_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_WND_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_thd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_THD_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_THD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_oc_wnd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OC_WND_MASK),
                             (kal_uint32)(PMIC_SPK_OC_WND_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_oc_thd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OC_THD_MASK),
                             (kal_uint32)(PMIC_SPK_OC_THD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_spk_d_oc_l_deg(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON6),
                           (&val),
                           (kal_uint32)(PMIC_SPK_D_OC_L_DEG_MASK),
                           (kal_uint32)(PMIC_SPK_D_OC_L_DEG_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_spk_ab_oc_l_deg(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(SPK_CON6),
                           (&val),
                           (kal_uint32)(PMIC_SPK_AB_OC_L_DEG_MASK),
                           (kal_uint32)(PMIC_SPK_AB_OC_L_DEG_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_spk_td1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TD1_MASK),
                             (kal_uint32)(PMIC_SPK_TD1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_td2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TD2_MASK),
                             (kal_uint32)(PMIC_SPK_TD2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_td3(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TD3_MASK),
                             (kal_uint32)(PMIC_SPK_TD3_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_div(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_DIV_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_DIV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_btl_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BTL_SET_MASK),
                             (kal_uint32)(PMIC_RG_BTL_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_ibias_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_IBIAS_SEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_IBIAS_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_ccode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_CCODE_MASK),
                             (kal_uint32)(PMIC_RG_SPK_CCODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_en_view_vcm(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_EN_VIEW_VCM_MASK),
                             (kal_uint32)(PMIC_RG_SPK_EN_VIEW_VCM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_en_view_clk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_EN_VIEW_CLK_MASK),
                             (kal_uint32)(PMIC_RG_SPK_EN_VIEW_CLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_vcm_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_VCM_SEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_VCM_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_vcm_ibsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_VCM_IBSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_VCM_IBSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_fbrc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_FBRC_EN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_FBRC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spkab_ovdrv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPKAB_OVDRV_MASK),
                             (kal_uint32)(PMIC_RG_SPKAB_OVDRV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_octh_d(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_OCTH_D_MASK),
                             (kal_uint32)(PMIC_RG_SPK_OCTH_D_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_RSV_MASK),
                             (kal_uint32)(PMIC_RG_SPK_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spkpga_gain(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPKPGA_GAIN_MASK),
                             (kal_uint32)(PMIC_RG_SPKPGA_GAIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_RSV0_MASK),
                             (kal_uint32)(PMIC_SPK_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_vcm_fast_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_VCM_FAST_EN_MASK),
                             (kal_uint32)(PMIC_SPK_VCM_FAST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_test_mode0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TEST_MODE0_MASK),
                             (kal_uint32)(PMIC_SPK_TEST_MODE0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_test_mode1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TEST_MODE1_MASK),
                             (kal_uint32)(PMIC_SPK_TEST_MODE1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_isense_refsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_REFSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_REFSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_isense_gainsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_GAINSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_GAINSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isense_pd_reset(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISENSE_PD_RESET_MASK),
                             (kal_uint32)(PMIC_RG_ISENSE_PD_RESET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_isense_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_EN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_isense_test_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_TEST_EN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_ISENSE_TEST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_td_wait(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TD_WAIT_MASK),
                             (kal_uint32)(PMIC_SPK_TD_WAIT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_td_done(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TD_DONE_MASK),
                             (kal_uint32)(PMIC_SPK_TD_DONE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_en_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_EN_MODE_MASK),
                             (kal_uint32)(PMIC_SPK_EN_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_vcm_fast_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_VCM_FAST_SW_MASK),
                             (kal_uint32)(PMIC_SPK_VCM_FAST_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_rst_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_RST_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_RST_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spkmode_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPKMODE_L_SW_MASK),
                             (kal_uint32)(PMIC_SPKMODE_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_depop_en_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_DEPOP_EN_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_DEPOP_EN_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_en_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_EN_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_EN_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_outstg_en_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_OUTSTG_EN_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_OUTSTG_EN_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_en_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_EN_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_EN_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_spk_trim_stop_l_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SPK_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SPK_TRIM_STOP_L_SW_MASK),
                             (kal_uint32)(PMIC_SPK_TRIM_STOP_L_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_cid(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CID),
                           (&val),
                           (kal_uint32)(PMIC_CID_MASK),
                           (kal_uint32)(PMIC_CID_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_clksq_en_aud(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUD_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_clksq_en_aux(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_clksq_en_fqr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_FQR_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_FQR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_75k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_75K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_75K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_32k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_32K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_32K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc_75k_div4_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC_75K_DIV4_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_RTC_75K_DIV4_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc_75k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC_75K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_RTC_75K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc_32k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC_32K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_RTC_32K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_32k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_32K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_32K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ldostb_1m_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LDOSTB_1M_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_LDOSTB_1M_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_intrp_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INTRP_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_INTRP_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_drv_32k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DRV_32K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_DRV_32K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck_1m_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK_1M_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_BUCK_1M_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_BUCK_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck_ana_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK_ANA_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_BUCK_ANA_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck32k_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK32K_PDN_MASK),
                             (kal_uint32)(PMIC_RG_BUCK32K_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_6m_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_6M_PDN_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_6M_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_pwm_div_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_PWM_DIV_PDN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_PWM_DIV_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_div_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_DIV_PDN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_DIV_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_SPK_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwmoc_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWMOC_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_PWMOC_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fqmtr_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FQMTR_PDN_MASK),
                             (kal_uint32)(PMIC_RG_FQMTR_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_drv_2m_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DRV_2M_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_DRV_2M_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_drv_1m_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DRV_1M_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_DRV_1M_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud_26m_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD_26M_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUD_26M_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_accdet_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ACCDET_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_ACCDET_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc_mclk_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC_MCLK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_RTC_MCLK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smps_ck_div_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMPS_CK_DIV_PDN_MASK),
                             (kal_uint32)(PMIC_RG_SMPS_CK_DIV_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc32k_1v8_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC32K_1V8_PDN_MASK),
                             (kal_uint32)(PMIC_RG_RTC32K_1V8_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_clksq_en_aux_md(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_ck_wake_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_WAKE_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_WAKE_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink0_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK0_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK0_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink1_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK1_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK1_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink2_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK2_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK2_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink3_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK3_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK3_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_ctl_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_CTL_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_CTL_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_32k_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_32K_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_32K_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud26m_div4_ck_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD26M_DIV4_CK_PDN_MASK),
                             (kal_uint32)(PMIC_RG_AUD26M_DIV4_CK_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_man_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_MAN_RST_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_MAN_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_RST_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audio_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDIO_RST_MASK),
                             (kal_uint32)(PMIC_RG_AUDIO_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_accdet_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ACCDET_RST_MASK),
                             (kal_uint32)(PMIC_RG_ACCDET_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_RST_MASK),
                             (kal_uint32)(PMIC_RG_SPK_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_driver_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DRIVER_RST_MASK),
                             (kal_uint32)(PMIC_RG_DRIVER_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC_RST_MASK),
                             (kal_uint32)(PMIC_RG_RTC_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fqmtr_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FQMTR_RST_MASK),
                             (kal_uint32)(PMIC_RG_FQMTR_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_top_rst_con_rsv_15_9(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TOP_RST_CON_RSV_15_9_MASK),
                             (kal_uint32)(PMIC_RG_TOP_RST_CON_RSV_15_9_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ap_rst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AP_RST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_AP_RST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_sysrstb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SYSRSTB_EN_MASK),
                             (kal_uint32)(PMIC_RG_SYSRSTB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_man_rst_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_MAN_RST_EN_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_MAN_RST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_newldo_rstb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_NEWLDO_RSTB_EN_MASK),
                             (kal_uint32)(PMIC_RG_NEWLDO_RSTB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rst_part_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RST_PART_SEL_MASK),
                             (kal_uint32)(PMIC_RG_RST_PART_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_homekey_rst_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_HOMEKEY_RST_EN_MASK),
                             (kal_uint32)(PMIC_RG_HOMEKEY_RST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwrkey_rst_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWRKEY_RST_EN_MASK),
                             (kal_uint32)(PMIC_RG_PWRKEY_RST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwrrst_tmr_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWRRST_TMR_DIS_MASK),
                             (kal_uint32)(PMIC_RG_PWRRST_TMR_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwrkey_rst_td(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_RST_MISC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWRKEY_RST_TD_MASK),
                             (kal_uint32)(PMIC_RG_PWRKEY_RST_TD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_srclken_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SRCLKEN_EN_MASK),
                             (kal_uint32)(PMIC_RG_SRCLKEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_osc_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OSC_SEL_MASK),
                             (kal_uint32)(PMIC_RG_OSC_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_sel_hw_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_SEL_HW_MODE_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_SEL_HW_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_srclken_hw_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SRCLKEN_HW_MODE_MASK),
                             (kal_uint32)(PMIC_RG_SRCLKEN_HW_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_osc_hw_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OSC_HW_MODE_MASK),
                             (kal_uint32)(PMIC_RG_OSC_HW_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_osc_hw_src_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OSC_HW_SRC_SEL_MASK),
                             (kal_uint32)(PMIC_RG_OSC_HW_SRC_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_ck_hw_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_HW_MODE_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_HW_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smps_autoff_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMPS_AUTOFF_DIS_MASK),
                             (kal_uint32)(PMIC_RG_SMPS_AUTOFF_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck_1m_autoff_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK_1M_AUTOFF_DIS_MASK),
                             (kal_uint32)(PMIC_RG_BUCK_1M_AUTOFF_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_buck_ana_autoff_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BUCK_ANA_AUTOFF_DIS_MASK),
                             (kal_uint32)(PMIC_RG_BUCK_ANA_AUTOFF_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_regck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_REGCK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_REGCK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_pwm_div_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_PWM_DIV_SEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_PWM_DIV_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_div_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_DIV_SEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_DIV_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fqmtr_cksel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FQMTR_CKSEL_MASK),
                             (kal_uint32)(PMIC_RG_FQMTR_CKSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_accdet_cksel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ACCDET_CKSEL_MASK),
                             (kal_uint32)(PMIC_RG_ACCDET_CKSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink0_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK0_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ISINK0_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink1_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK1_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ISINK1_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink2_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK2_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ISINK2_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink3_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK3_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ISINK3_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audio_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKCON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDIO_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_AUDIO_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc32k_tst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC32K_TST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_RTC32K_TST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_tst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_TST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_SPK_TST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smps_tst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMPS_TST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_SMPS_TST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pmu75k_tst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PMU75K_TST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_PMU75K_TST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud26m_tst_dis(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD26M_TST_DIS_MASK),
                             (kal_uint32)(PMIC_RG_AUD26M_TST_DIS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spk_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPK_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPK_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smps_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMPS_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_SMPS_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtc32k_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTC32K_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_RTC32K_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pmu75k_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PMU75K_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_PMU75K_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud26m_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD26M_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUD26M_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rtcdet_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RTCDET_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_RTCDET_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwmoc_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWMOC_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_PWMOC_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ldostb_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LDOSTB_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_LDOSTB_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_ISINK_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fqmtr_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FQMTR_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_FQMTR_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_classd_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLASSD_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_CLASSD_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_sdm_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_SDM_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud26m_div4_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD26M_DIV4_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUD26M_DIV4_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audif_tstsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDIF_TSTSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUDIF_TSTSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_test_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TEST_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TEST_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pchr_test_ck_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PCHR_TEST_CK_SEL_MASK),
                             (kal_uint32)(PMIC_RG_PCHR_TEST_CK_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_strup_75k_26m_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STRUP_75K_26M_SEL_MASK),
                             (kal_uint32)(PMIC_RG_STRUP_75K_26M_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_bgr_testmode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BGR_TESTMODE_MASK),
                             (kal_uint32)(PMIC_RG_BGR_TESTMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_top_cktst2_rsv_15_8(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TOP_CKTST2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TOP_CKTST2_RSV_15_8_MASK),
                             (kal_uint32)(PMIC_RG_TOP_CKTST2_RSV_15_8_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_test_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(TEST_OUT),
                           (&val),
                           (kal_uint32)(PMIC_TEST_OUT_MASK),
                           (kal_uint32)(PMIC_TEST_OUT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_mon_flag_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_MON_FLAG_SEL_MASK),
                             (kal_uint32)(PMIC_RG_MON_FLAG_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_mon_grp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_MON_GRP_SEL_MASK),
                             (kal_uint32)(PMIC_RG_MON_GRP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_driver(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_DRIVER_MASK),
                             (kal_uint32)(PMIC_RG_TEST_DRIVER_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_classd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_CLASSD_MASK),
                             (kal_uint32)(PMIC_RG_TEST_CLASSD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_aud(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_AUD_MASK),
                             (kal_uint32)(PMIC_RG_TEST_AUD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_auxadc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_AUXADC_MASK),
                             (kal_uint32)(PMIC_RG_TEST_AUXADC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_nandtree_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_NANDTREE_MODE_MASK),
                             (kal_uint32)(PMIC_RG_NANDTREE_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_MODE_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_strup(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_STRUP_MASK),
                             (kal_uint32)(PMIC_RG_TEST_STRUP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_spk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_SPK_MASK),
                             (kal_uint32)(PMIC_RG_TEST_SPK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_test_spk_pwm(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TEST_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TEST_SPK_PWM_MASK),
                             (kal_uint32)(PMIC_RG_TEST_SPK_PWM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_en_status_vproc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VPROC_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VPROC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vsys(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VSYS_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VSYS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vpa(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VPA_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VPA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vrtc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VRTC_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VRTC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_va(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VA_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcama(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCAMA_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCAMA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcamd(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCAMD_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCAMD_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcam_af(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCAM_AF_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCAM_AF_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcam_io(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCAM_IO_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCAM_IO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcn28(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCN28_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCN28_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcn33(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCN33_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCN33_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vcn_1v8(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VCN_1V8_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VCN_1V8_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vemc_3v3(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VEMC_3V3_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VEMC_3V3_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vgp1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VGP1_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VGP1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vgp2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VGP2_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VGP2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vgp3(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VGP3_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VGP3_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vibr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VIBR_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VIBR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vio18(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VIO18_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VIO18_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vio28(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VIO28_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VIO28_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vm(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VM_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VM_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vmc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VMC_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VMC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vmch(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VMCH_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VMCH_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vrf18(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VRF18_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VRF18_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vsim1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VSIM1_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VSIM1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vsim2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VSIM2_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VSIM2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vtcxo(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VTCXO_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VTCXO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_en_status_vusb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EN_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_EN_STATUS_VUSB_MASK),
                           (kal_uint32)(PMIC_EN_STATUS_VUSB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vproc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VPROC_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VPROC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vsys(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VSYS_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VSYS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vpa(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VPA_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VPA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_va(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VA_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcama(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCAMA_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCAMA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcamd(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCAMD_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCAMD_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcam_af(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCAM_AF_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCAM_AF_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcam_io(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCAM_IO_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCAM_IO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcn28(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCN28_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCN28_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcn33(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCN33_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCN33_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vcn_1v8(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VCN_1V8_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VCN_1V8_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vemc_3v3(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VEMC_3V3_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VEMC_3V3_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vgp1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VGP1_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VGP1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vgp2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VGP2_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VGP2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vgp3(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS0),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VGP3_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VGP3_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vibr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VIBR_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VIBR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vio18(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VIO18_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VIO18_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vio28(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VIO28_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VIO28_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vm(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VM_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VM_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vmc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VMC_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VMC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vmch(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VMCH_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VMCH_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vrf18(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VRF18_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VRF18_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vsim1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VSIM1_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VSIM1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vsim2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VSIM2_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VSIM2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vtcxo(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VTCXO_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VTCXO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_oc_status_vusb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_OC_STATUS_VUSB_MASK),
                           (kal_uint32)(PMIC_OC_STATUS_VUSB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_spk_oc_det_d_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_NI_SPK_OC_DET_D_L_MASK),
                           (kal_uint32)(PMIC_NI_SPK_OC_DET_D_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_spk_oc_det_ab_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(OCSTATUS1),
                           (&val),
                           (kal_uint32)(PMIC_NI_SPK_OC_DET_AB_L_MASK),
                           (kal_uint32)(PMIC_NI_SPK_OC_DET_AB_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vproc_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VPROC_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VPROC_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vsys_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VSYS_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VSYS_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vm_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VM_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VM_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vio18_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VIO18_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VIO18_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vtcxo_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VTCXO_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VTCXO_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_va_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VA_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VA_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vio28_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VIO28_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VIO28_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vgp2_pg_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(PGSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VGP2_PG_DEB_MASK),
                           (kal_uint32)(PMIC_VGP2_PG_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_pmu_test_mode_scan(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_PMU_TEST_MODE_SCAN_MASK),
                           (kal_uint32)(PMIC_PMU_TEST_MODE_SCAN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_pwrkey_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_PWRKEY_DEB_MASK),
                           (kal_uint32)(PMIC_PWRKEY_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_fchrkey_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_FCHRKEY_DEB_MASK),
                           (kal_uint32)(PMIC_FCHRKEY_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_vbat_ov(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_VBAT_OV_MASK),
                           (kal_uint32)(PMIC_VBAT_OV_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_pchr_chrdet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_PCHR_CHRDET_MASK),
                           (kal_uint32)(PMIC_PCHR_CHRDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ro_baton_undet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_RO_BATON_UNDET_MASK),
                           (kal_uint32)(PMIC_RO_BATON_UNDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rtc_xtal_det_done(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_RTC_XTAL_DET_DONE_MASK),
                           (kal_uint32)(PMIC_RTC_XTAL_DET_DONE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_xosc32_enb_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHRSTATUS),
                           (&val),
                           (kal_uint32)(PMIC_XOSC32_ENB_DET_MASK),
                           (kal_uint32)(PMIC_XOSC32_ENB_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rtc_xtal_det_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHRSTATUS),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RTC_XTAL_DET_RSV_MASK),
                             (kal_uint32)(PMIC_RTC_XTAL_DET_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simap_tdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMAP_TDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SIMAP_TDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud_tdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD_TDSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUD_TDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spi_tdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPI_TDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPI_TDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pmu_tdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PMU_TDSEL_MASK),
                             (kal_uint32)(PMIC_RG_PMU_TDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls_tdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(TDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS_TDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS_TDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simap_rdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMAP_RDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SIMAP_RDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud_rdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD_RDSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUD_RDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_spi_rdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPI_RDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SPI_RDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pmu_rdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PMU_RDSEL_MASK),
                             (kal_uint32)(PMIC_RG_PMU_RDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls_rdsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RDSEL_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS_RDSEL_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS_RDSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_sysrstb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SYSRSTB_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SYSRSTB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_int(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_INT_MASK),
                             (kal_uint32)(PMIC_RG_SMT_INT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_srclken(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SRCLKEN_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SRCLKEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_rtc_32k1v8(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_RTC_32K1V8_MASK),
                             (kal_uint32)(PMIC_RG_SMT_RTC_32K1V8_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_spi_clk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SPI_CLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SPI_CLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_spi_csn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SPI_CSN_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SPI_CSN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_spi_mosi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SPI_MOSI_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SPI_MOSI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_spi_miso(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SPI_MISO_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SPI_MISO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_aud_clk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_AUD_CLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_AUD_CLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_aud_mosi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_AUD_MOSI_MASK),
                             (kal_uint32)(PMIC_RG_SMT_AUD_MOSI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_aud_miso(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_AUD_MISO_MASK),
                             (kal_uint32)(PMIC_RG_SMT_AUD_MISO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_sim1_ap_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIM1_AP_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIM1_AP_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_sim1_ap_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIM1_AP_SRST_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIM1_AP_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_simls1_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS1_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS1_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_simls1_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS1_SRST_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS1_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_sim2_ap_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIM2_AP_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIM2_AP_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_sim2_ap_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIM2_AP_SRST_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIM2_AP_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_simls2_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS2_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS2_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smt_simls2_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SMT_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS2_SRST_MASK),
                             (kal_uint32)(PMIC_RG_SMT_SIMLS2_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_int(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_INT_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_INT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_srclken(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SRCLKEN_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SRCLKEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_rtc_32k1v8(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_RTC_32K1V8_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_RTC_32K1V8_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_spi_clk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_CLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_CLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_spi_csn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_CSN_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_CSN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_spi_mosi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_MOSI_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_MOSI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_spi_miso(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_MISO_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SPI_MISO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_aud_clk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_CLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_CLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_aud_mosi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_MOSI_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_MOSI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_aud_miso(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_MISO_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_AUD_MISO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_sim1_ap_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIM1_AP_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIM1_AP_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_sim1_ap_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIM1_AP_SRST_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIM1_AP_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_simls1_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS1_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS1_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_simls1_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS1_SRST_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS1_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_sim2_ap_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIM2_AP_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIM2_AP_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_sim2_ap_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIM2_AP_SRST_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIM2_AP_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_simls2_sclk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS2_SCLK_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS2_SCLK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_octl_simls2_srst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DRV_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS2_SRST_MASK),
                             (kal_uint32)(PMIC_RG_OCTL_SIMLS2_SRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls1_sclk_conf(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SIMLS1_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS1_SCLK_CONF_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS1_SCLK_CONF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls1_srst_conf(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SIMLS1_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS1_SRST_CONF_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS1_SRST_CONF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls2_sclk_conf(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SIMLS2_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS2_SCLK_CONF_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS2_SCLK_CONF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_simls2_srst_conf(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(SIMLS2_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SIMLS2_SRST_CONF_MASK),
                             (kal_uint32)(PMIC_RG_SIMLS2_SRST_CONF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_spkl_ab(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_SPKL_AB_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_SPKL_AB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_spkl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_SPKL_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_SPKL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_bat_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_BAT_L_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_BAT_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_bat_h(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_BAT_H_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_BAT_H_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_watchdog(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_WATCHDOG_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_WATCHDOG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_pwrkey(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_PWRKEY_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_PWRKEY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_thr_l(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_THR_L_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_THR_L_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_thr_h(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_THR_H_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_THR_H_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_vbaton_undet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_VBATON_UNDET_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_VBATON_UNDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_bvalid_det(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_BVALID_DET_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_BVALID_DET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_chrdet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_CHRDET_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_CHRDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_ov(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_OV_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_OV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_ldo(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_LDO_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_LDO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_fchrkey(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_FCHRKEY_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_FCHRKEY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_accdet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_ACCDET_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_ACCDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_audio(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_AUDIO_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_AUDIO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_rtc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_RTC_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_RTC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_vproc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_VPROC_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_VPROC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_vsys(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_VSYS_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_VSYS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_int_en_vpa(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_INT_EN_VPA_MASK),
                             (kal_uint32)(PMIC_RG_INT_EN_VPA_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_polarity(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_POLARITY_MASK),
                             (kal_uint32)(PMIC_POLARITY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_polarity_vbaton_undet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_POLARITY_VBATON_UNDET_MASK),
                             (kal_uint32)(PMIC_POLARITY_VBATON_UNDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_polarity_bvalid_det(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_POLARITY_BVALID_DET_MASK),
                             (kal_uint32)(PMIC_POLARITY_BVALID_DET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_fchrkey_int_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_FCHRKEY_INT_SEL_MASK),
                             (kal_uint32)(PMIC_RG_FCHRKEY_INT_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_pwrkey_int_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_PWRKEY_INT_SEL_MASK),
                             (kal_uint32)(PMIC_RG_PWRKEY_INT_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_ivgen_ext_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(INT_MISC_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_IVGEN_EXT_EN_MASK),
                             (kal_uint32)(PMIC_IVGEN_EXT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_int_status_spkl_ab(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_SPKL_AB_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_SPKL_AB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_spkl(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_SPKL_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_SPKL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_bat_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BAT_L_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BAT_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_bat_h(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BAT_H_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BAT_H_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_watchdog(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_WATCHDOG_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_WATCHDOG_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_pwrkey(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_PWRKEY_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_PWRKEY_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_thr_l(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_THR_L_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_THR_L_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_thr_h(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_THR_H_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_THR_H_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_vbaton_undet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VBATON_UNDET_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VBATON_UNDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_bvalid_det(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BVALID_DET_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_BVALID_DET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_chrdet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_CHRDET_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_CHRDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_ov(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS0),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_OV_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_OV_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_ldo(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_LDO_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_LDO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_fchrkey(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_FCHRKEY_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_FCHRKEY_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_accdet(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_ACCDET_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_ACCDET_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_audio(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_AUDIO_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_AUDIO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_rtc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_RTC_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_RTC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_vproc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VPROC_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VPROC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_vsys(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VSYS_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VSYS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_int_status_vpa(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(INT_STATUS1),
                           (&val),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VPA_MASK),
                           (kal_uint32)(PMIC_RG_INT_STATUS_VPA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_oc_gear_bvalid_det(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_GEAR_0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_OC_GEAR_BVALID_DET_MASK),
                             (kal_uint32)(PMIC_OC_GEAR_BVALID_DET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_oc_gear_vbaton_undet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_GEAR_1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_OC_GEAR_VBATON_UNDET_MASK),
                             (kal_uint32)(PMIC_OC_GEAR_VBATON_UNDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_oc_gear_ldo(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_GEAR_2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_OC_GEAR_LDO_MASK),
                             (kal_uint32)(PMIC_OC_GEAR_LDO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_oc_thd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPROC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_OC_THD_MASK),
                             (kal_uint32)(PMIC_VPROC_OC_THD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_oc_wnd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPROC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_OC_WND_MASK),
                             (kal_uint32)(PMIC_VPROC_OC_WND_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_deg_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPROC),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DEG_EN_MASK),
                             (kal_uint32)(PMIC_VPROC_DEG_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_oc_thd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VSYS),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_OC_THD_MASK),
                             (kal_uint32)(PMIC_VSYS_OC_THD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_oc_wnd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VSYS),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_OC_WND_MASK),
                             (kal_uint32)(PMIC_VSYS_OC_WND_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_deg_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VSYS),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DEG_EN_MASK),
                             (kal_uint32)(PMIC_VSYS_DEG_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_oc_thd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPA),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_OC_THD_MASK),
                             (kal_uint32)(PMIC_VPA_OC_THD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_oc_wnd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPA),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_OC_WND_MASK),
                             (kal_uint32)(PMIC_VPA_OC_WND_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_deg_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(OC_CTL_VPA),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DEG_EN_MASK),
                             (kal_uint32)(PMIC_VPA_DEG_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_fqmtr_tcksel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(FQMTR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_FQMTR_TCKSEL_MASK),
                             (kal_uint32)(PMIC_FQMTR_TCKSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_fqmtr_busy(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(FQMTR_CON0),
                           (&val),
                           (kal_uint32)(PMIC_FQMTR_BUSY_MASK),
                           (kal_uint32)(PMIC_FQMTR_BUSY_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_fqmtr_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(FQMTR_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_FQMTR_EN_MASK),
                             (kal_uint32)(PMIC_FQMTR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_fqmtr_winset(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(FQMTR_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_FQMTR_WINSET_MASK),
                             (kal_uint32)(PMIC_FQMTR_WINSET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_fqmtr_data(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(FQMTR_CON2),
                           (&val),
                           (kal_uint32)(PMIC_FQMTR_DATA_MASK),
                           (kal_uint32)(PMIC_FQMTR_DATA_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_spi_con(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RG_SPI_CON),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SPI_CON_MASK),
                             (kal_uint32)(PMIC_RG_SPI_CON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_dio_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_DIO_EN),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_DIO_EN_MASK),
                             (kal_uint32)(PMIC_DEW_DIO_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_dew_read_test(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DEW_READ_TEST),
                           (&val),
                           (kal_uint32)(PMIC_DEW_READ_TEST_MASK),
                           (kal_uint32)(PMIC_DEW_READ_TEST_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_dew_write_test(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_WRITE_TEST),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_WRITE_TEST_MASK),
                             (kal_uint32)(PMIC_DEW_WRITE_TEST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_crc_swrst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CRC_SWRST),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CRC_SWRST_MASK),
                             (kal_uint32)(PMIC_DEW_CRC_SWRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_crc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CRC_EN),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CRC_EN_MASK),
                             (kal_uint32)(PMIC_DEW_CRC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_dew_crc_val(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DEW_CRC_VAL),
                           (&val),
                           (kal_uint32)(PMIC_DEW_CRC_VAL_MASK),
                           (kal_uint32)(PMIC_DEW_CRC_VAL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_dew_dbg_mon_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_DBG_MON_SEL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_DBG_MON_SEL_MASK),
                             (kal_uint32)(PMIC_DEW_DBG_MON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_cipher_key_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CIPHER_KEY_SEL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CIPHER_KEY_SEL_MASK),
                             (kal_uint32)(PMIC_DEW_CIPHER_KEY_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_cipher_iv_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CIPHER_IV_SEL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CIPHER_IV_SEL_MASK),
                             (kal_uint32)(PMIC_DEW_CIPHER_IV_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_cipher_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CIPHER_EN),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CIPHER_EN_MASK),
                             (kal_uint32)(PMIC_DEW_CIPHER_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_dew_cipher_rdy(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DEW_CIPHER_RDY),
                           (&val),
                           (kal_uint32)(PMIC_DEW_CIPHER_RDY_MASK),
                           (kal_uint32)(PMIC_DEW_CIPHER_RDY_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_dew_cipher_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CIPHER_MODE),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CIPHER_MODE_MASK),
                             (kal_uint32)(PMIC_DEW_CIPHER_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_cipher_swrst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_CIPHER_SWRST),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_CIPHER_SWRST_MASK),
                             (kal_uint32)(PMIC_DEW_CIPHER_SWRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_rddmy_no(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_RDDMY_NO),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_RDDMY_NO_MASK),
                             (kal_uint32)(PMIC_DEW_RDDMY_NO_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_dew_rdata_dly_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DEW_RDATA_DLY_SEL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DEW_RDATA_DLY_SEL_MASK),
                             (kal_uint32)(PMIC_DEW_RDATA_DLY_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_smps_testmode_b(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SMPS_TESTMODE_B_MASK),
                             (kal_uint32)(PMIC_RG_SMPS_TESTMODE_B_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vproc_dig_mon(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_DIG_MON_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_DIG_MON_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vsys_dig_mon(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_CON1),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_DIG_MON_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_DIG_MON_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsleep_src0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSLEEP_SRC0_MASK),
                             (kal_uint32)(PMIC_VSLEEP_SRC0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsleep_src1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSLEEP_SRC1_MASK),
                             (kal_uint32)(PMIC_VSLEEP_SRC1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_r2r_src0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_R2R_SRC0_MASK),
                             (kal_uint32)(PMIC_R2R_SRC0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_r2r_src1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_R2R_SRC1_MASK),
                             (kal_uint32)(PMIC_R2R_SRC1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_buck_osc_sel_src0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BUCK_OSC_SEL_SRC0_MASK),
                             (kal_uint32)(PMIC_BUCK_OSC_SEL_SRC0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_srclken_dly_src1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_SRCLKEN_DLY_SRC1_MASK),
                             (kal_uint32)(PMIC_SRCLKEN_DLY_SRC1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_buck_con5_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_BUCK_CON5_RSV0_MASK),
                             (kal_uint32)(PMIC_BUCK_CON5_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_triml(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_TRIML_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_TRIML_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_trimh(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_TRIMH_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_TRIMH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_csm(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_CSM_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_CSM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_zxos_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_ZXOS_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_ZXOS_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_rzsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_RZSEL_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_RZSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_cc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_CC_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_CC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_csr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_CSR_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_CSR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_csl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_CSL_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_CSL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_zx_os(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_ZX_OS_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_ZX_OS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_avp_os(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_AVP_OS_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_AVP_OS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_avp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_AVP_EN_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_AVP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_modeset(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_MODESET_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_MODESET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_slp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_SLP_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_SLP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_qi_vproc_vsleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_QI_VPROC_VSLEEP_MASK),
                             (kal_uint32)(PMIC_QI_VPROC_VSLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vproc_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPROC_RSV_MASK),
                             (kal_uint32)(PMIC_RG_VPROC_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_en_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_EN_CTRL_MASK),
                             (kal_uint32)(PMIC_VPROC_EN_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_CTRL_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_dlc_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_CTRL_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_burst_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_BURST_CTRL_MASK),
                             (kal_uint32)(PMIC_VPROC_BURST_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_EN_MASK),
                             (kal_uint32)(PMIC_VPROC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vproc_stb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_STB_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_STB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vproc_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_EN_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vproc_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_sfchg_frate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_SFCHG_FRATE_MASK),
                             (kal_uint32)(PMIC_VPROC_SFCHG_FRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_sfchg_fen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_SFCHG_FEN_MASK),
                             (kal_uint32)(PMIC_VPROC_SFCHG_FEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_sfchg_rrate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_SFCHG_RRATE_MASK),
                             (kal_uint32)(PMIC_VPROC_SFCHG_RRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_sfchg_ren(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_SFCHG_REN_MASK),
                             (kal_uint32)(PMIC_VPROC_SFCHG_REN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_ON_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vproc_vosel(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON12),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPROC_VOSEL_MASK),
                           (kal_uint32)(PMIC_NI_VPROC_VOSEL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_burst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_BURST_MASK),
                             (kal_uint32)(PMIC_VPROC_BURST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_burst_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_BURST_ON_MASK),
                             (kal_uint32)(PMIC_VPROC_BURST_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_burst_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_BURST_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPROC_BURST_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vproc_burst(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON13),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_BURST_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_BURST_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_dlc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_dlc_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_ON_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_dlc_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vproc_dlc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON14),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_DLC_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_DLC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_dlc_n(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_N_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_N_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_dlc_n_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_N_ON_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_N_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_dlc_n_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_DLC_N_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPROC_DLC_N_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vproc_dlc_n(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON15),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPROC_DLC_N_MASK),
                           (kal_uint32)(PMIC_QI_VPROC_DLC_N_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_transtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_TRANSTD_MASK),
                             (kal_uint32)(PMIC_VPROC_TRANSTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel_trans_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_TRANS_EN_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_TRANS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vosel_trans_once(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VOSEL_TRANS_ONCE_MASK),
                             (kal_uint32)(PMIC_VPROC_VOSEL_TRANS_ONCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vproc_vosel_trans(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPROC_VOSEL_TRANS_MASK),
                           (kal_uint32)(PMIC_NI_VPROC_VOSEL_TRANS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vproc_vsleep_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VSLEEP_EN_MASK),
                             (kal_uint32)(PMIC_VPROC_VSLEEP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_r2r_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_R2R_PDN_MASK),
                             (kal_uint32)(PMIC_VPROC_R2R_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vproc_vsleep_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPROC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPROC_VSLEEP_SEL_MASK),
                             (kal_uint32)(PMIC_VPROC_VSLEEP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vproc_r2r_pdn(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPROC_R2R_PDN_MASK),
                           (kal_uint32)(PMIC_NI_VPROC_R2R_PDN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_vproc_vsleep_sel(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPROC_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPROC_VSLEEP_SEL_MASK),
                           (kal_uint32)(PMIC_NI_VPROC_VSLEEP_SEL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vsys_triml(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_TRIML_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_TRIML_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_trimh(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_TRIMH_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_TRIMH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_csm(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_CSM_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_CSM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_zxos_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_ZXOS_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_ZXOS_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_rzsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_RZSEL_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_RZSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_cc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_CC_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_CC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_csr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_CSR_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_CSR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_csl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_CSL_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_CSL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_zx_os(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_ZX_OS_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_ZX_OS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_avp_os(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_AVP_OS_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_AVP_OS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_avp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_AVP_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_AVP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_modeset(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_MODESET_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_MODESET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_slp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_SLP_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_SLP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_qi_vsys_vsleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_QI_VSYS_VSLEEP_MASK),
                             (kal_uint32)(PMIC_QI_VSYS_VSLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsys_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYS_RSV_MASK),
                             (kal_uint32)(PMIC_RG_VSYS_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_en_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_EN_CTRL_MASK),
                             (kal_uint32)(PMIC_VSYS_EN_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_CTRL_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_dlc_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_CTRL_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_burst_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_BURST_CTRL_MASK),
                             (kal_uint32)(PMIC_VSYS_BURST_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_EN_MASK),
                             (kal_uint32)(PMIC_VSYS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsys_stb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_STB_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_STB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vsys_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_EN_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vsys_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_sfchg_frate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_SFCHG_FRATE_MASK),
                             (kal_uint32)(PMIC_VSYS_SFCHG_FRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_sfchg_fen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_SFCHG_FEN_MASK),
                             (kal_uint32)(PMIC_VSYS_SFCHG_FEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_sfchg_rrate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_SFCHG_RRATE_MASK),
                             (kal_uint32)(PMIC_VSYS_SFCHG_RRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_sfchg_ren(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_SFCHG_REN_MASK),
                             (kal_uint32)(PMIC_VSYS_SFCHG_REN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_ON_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_SLEEP_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vsys_vosel(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON12),
                           (&val),
                           (kal_uint32)(PMIC_NI_VSYS_VOSEL_MASK),
                           (kal_uint32)(PMIC_NI_VSYS_VOSEL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_burst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_BURST_MASK),
                             (kal_uint32)(PMIC_VSYS_BURST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_burst_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_BURST_ON_MASK),
                             (kal_uint32)(PMIC_VSYS_BURST_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_burst_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_BURST_SLEEP_MASK),
                             (kal_uint32)(PMIC_VSYS_BURST_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsys_burst(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON13),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_BURST_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_BURST_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_dlc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_dlc_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_ON_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_dlc_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_SLEEP_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsys_dlc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON14),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_DLC_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_DLC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_dlc_n(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_N_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_N_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_dlc_n_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_N_ON_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_N_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_dlc_n_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_DLC_N_SLEEP_MASK),
                             (kal_uint32)(PMIC_VSYS_DLC_N_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsys_dlc_n(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON15),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSYS_DLC_N_MASK),
                           (kal_uint32)(PMIC_QI_VSYS_DLC_N_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_transtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_TRANSTD_MASK),
                             (kal_uint32)(PMIC_VSYS_TRANSTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel_trans_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_TRANS_EN_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_TRANS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vosel_trans_once(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VOSEL_TRANS_ONCE_MASK),
                             (kal_uint32)(PMIC_VSYS_VOSEL_TRANS_ONCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vsys_vosel_trans(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VSYS_VOSEL_TRANS_MASK),
                           (kal_uint32)(PMIC_NI_VSYS_VOSEL_TRANS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vsys_vsleep_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VSLEEP_EN_MASK),
                             (kal_uint32)(PMIC_VSYS_VSLEEP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_r2r_pdn(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_R2R_PDN_MASK),
                             (kal_uint32)(PMIC_VSYS_R2R_PDN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsys_vsleep_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VSYS_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSYS_VSLEEP_SEL_MASK),
                             (kal_uint32)(PMIC_VSYS_VSLEEP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vsys_r2r_pdn(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VSYS_R2R_PDN_MASK),
                           (kal_uint32)(PMIC_NI_VSYS_R2R_PDN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_vsys_vsleep_sel(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VSYS_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VSYS_VSLEEP_SEL_MASK),
                           (kal_uint32)(PMIC_NI_VSYS_VSLEEP_SEL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vpa_triml(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_TRIML_MASK),
                             (kal_uint32)(PMIC_RG_VPA_TRIML_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_trimh(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_TRIMH_MASK),
                             (kal_uint32)(PMIC_RG_VPA_TRIMH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_rzsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_RZSEL_MASK),
                             (kal_uint32)(PMIC_RG_VPA_RZSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_cc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_CC_MASK),
                             (kal_uint32)(PMIC_RG_VPA_CC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_csr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_CSR_MASK),
                             (kal_uint32)(PMIC_RG_VPA_CSR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_csl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_CSL_MASK),
                             (kal_uint32)(PMIC_RG_VPA_CSL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_slew_nmos(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_SLEW_NMOS_MASK),
                             (kal_uint32)(PMIC_RG_VPA_SLEW_NMOS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_slew(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_SLEW_MASK),
                             (kal_uint32)(PMIC_RG_VPA_SLEW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_zx_os(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_ZX_OS_MASK),
                             (kal_uint32)(PMIC_RG_VPA_ZX_OS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_modeset(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_MODESET_MASK),
                             (kal_uint32)(PMIC_RG_VPA_MODESET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VPA_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_csmir(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_CSMIR_MASK),
                             (kal_uint32)(PMIC_RG_VPA_CSMIR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_vbat_del(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_VBAT_DEL_MASK),
                             (kal_uint32)(PMIC_RG_VPA_VBAT_DEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_slp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_SLP_MASK),
                             (kal_uint32)(PMIC_RG_VPA_SLP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_gpu_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_GPU_EN_MASK),
                             (kal_uint32)(PMIC_RG_VPA_GPU_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpa_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPA_RSV_MASK),
                             (kal_uint32)(PMIC_RG_VPA_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_en_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_EN_CTRL_MASK),
                             (kal_uint32)(PMIC_VPA_EN_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_CTRL_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_dlc_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DLC_CTRL_MASK),
                             (kal_uint32)(PMIC_VPA_DLC_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_burst_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURST_CTRL_MASK),
                             (kal_uint32)(PMIC_VPA_BURST_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_EN_MASK),
                             (kal_uint32)(PMIC_VPA_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vpa_stb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_STB_MASK),
                           (kal_uint32)(PMIC_QI_VPA_STB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vpa_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_EN_MASK),
                           (kal_uint32)(PMIC_QI_VPA_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vpa_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VPA_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_sfchg_frate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_SFCHG_FRATE_MASK),
                             (kal_uint32)(PMIC_VPA_SFCHG_FRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_sfchg_fen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_SFCHG_FEN_MASK),
                             (kal_uint32)(PMIC_VPA_SFCHG_FEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_sfchg_rrate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_SFCHG_RRATE_MASK),
                             (kal_uint32)(PMIC_VPA_SFCHG_RRATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_sfchg_ren(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_SFCHG_REN_MASK),
                             (kal_uint32)(PMIC_VPA_SFCHG_REN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_ON_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vpa_vosel(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON12),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPA_VOSEL_MASK),
                           (kal_uint32)(PMIC_NI_VPA_VOSEL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_dlc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DLC_MASK),
                             (kal_uint32)(PMIC_VPA_DLC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_dlc_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DLC_ON_MASK),
                             (kal_uint32)(PMIC_VPA_DLC_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_dlc_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DLC_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPA_DLC_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vpa_dlc(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON14),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_DLC_MASK),
                           (kal_uint32)(PMIC_QI_VPA_DLC_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_bursth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTH_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_bursth_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTH_ON_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTH_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_bursth_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTH_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTH_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vpa_bursth(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON16),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_BURSTH_MASK),
                           (kal_uint32)(PMIC_QI_VPA_BURSTH_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_burstl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTL_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_burstl_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTL_ON_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTL_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_burstl_sleep(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_BURSTL_SLEEP_MASK),
                             (kal_uint32)(PMIC_VPA_BURSTL_SLEEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vpa_burstl(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON17),
                           (&val),
                           (kal_uint32)(PMIC_QI_VPA_BURSTL_MASK),
                           (kal_uint32)(PMIC_QI_VPA_BURSTL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_transtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_TRANSTD_MASK),
                             (kal_uint32)(PMIC_VPA_TRANSTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_trans_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_TRANS_EN_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_TRANS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_trans_once(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_TRANS_ONCE_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_TRANS_ONCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_vpa_dvs_bw(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(VPA_CON18),
                           (&val),
                           (kal_uint32)(PMIC_NI_VPA_DVS_BW_MASK),
                           (kal_uint32)(PMIC_NI_VPA_DVS_BW_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vpa_dlc_map_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_DLC_MAP_EN_MASK),
                             (kal_uint32)(PMIC_VPA_DLC_MAP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_dlc001(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC001_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC001_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_dlc011(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC011_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC011_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vpa_vosel_dlc111(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(VPA_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC111_MASK),
                             (kal_uint32)(PMIC_VPA_VOSEL_DLC111_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_rst_done(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_RST_DONE_MASK),
                             (kal_uint32)(PMIC_K_RST_DONE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_map_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_MAP_SEL_MASK),
                             (kal_uint32)(PMIC_K_MAP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_once_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_ONCE_EN_MASK),
                             (kal_uint32)(PMIC_K_ONCE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_once(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_ONCE_MASK),
                             (kal_uint32)(PMIC_K_ONCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_start_manual(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_START_MANUAL_MASK),
                             (kal_uint32)(PMIC_K_START_MANUAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_src_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_SRC_SEL_MASK),
                             (kal_uint32)(PMIC_K_SRC_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_auto_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_AUTO_EN_MASK),
                             (kal_uint32)(PMIC_K_AUTO_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_inv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_INV_MASK),
                             (kal_uint32)(PMIC_K_INV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_k_control_smps(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(BUCK_K_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_K_CONTROL_SMPS_MASK),
                             (kal_uint32)(PMIC_K_CONTROL_SMPS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_k_result(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_K_CON2),
                           (&val),
                           (kal_uint32)(PMIC_K_RESULT_MASK),
                           (kal_uint32)(PMIC_K_RESULT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_k_done(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_K_CON2),
                           (&val),
                           (kal_uint32)(PMIC_K_DONE_MASK),
                           (kal_uint32)(PMIC_K_DONE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_k_control(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_K_CON2),
                           (&val),
                           (kal_uint32)(PMIC_K_CONTROL_MASK),
                           (kal_uint32)(PMIC_K_CONTROL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_smps_osc_cal(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(BUCK_K_CON2),
                           (&val),
                           (kal_uint32)(PMIC_QI_SMPS_OSC_CAL_MASK),
                           (kal_uint32)(PMIC_QI_SMPS_OSC_CAL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_isink_ch0_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH0_MODE_MASK),
                             (kal_uint32)(PMIC_ISINK_CH0_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink0_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK0_RSV1_MASK),
                             (kal_uint32)(PMIC_ISINK0_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim0_duty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM0_DUTY_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM0_DUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink0_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK0_RSV0_MASK),
                             (kal_uint32)(PMIC_ISINK0_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim0_fsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM0_FSEL_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM0_FSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr0_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR0_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR0_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr0_tc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR0_TC_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR0_TC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch0_step(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH0_STEP_MASK),
                             (kal_uint32)(PMIC_ISINK_CH0_STEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath0_toff_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TOFF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TOFF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath0_ton_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TON_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath0_trf_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK0_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TRF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH0_TRF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch1_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH1_MODE_MASK),
                             (kal_uint32)(PMIC_ISINK_CH1_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink1_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK1_RSV1_MASK),
                             (kal_uint32)(PMIC_ISINK1_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim1_duty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM1_DUTY_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM1_DUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink1_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK1_RSV0_MASK),
                             (kal_uint32)(PMIC_ISINK1_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim1_fsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM1_FSEL_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM1_FSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr1_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR1_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR1_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr1_tc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR1_TC_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR1_TC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch1_step(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH1_STEP_MASK),
                             (kal_uint32)(PMIC_ISINK_CH1_STEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath1_toff_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TOFF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TOFF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath1_ton_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TON_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath1_trf_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK1_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TRF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH1_TRF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch2_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH2_MODE_MASK),
                             (kal_uint32)(PMIC_ISINK_CH2_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink2_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK2_RSV1_MASK),
                             (kal_uint32)(PMIC_ISINK2_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim2_duty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM2_DUTY_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM2_DUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink2_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK2_RSV0_MASK),
                             (kal_uint32)(PMIC_ISINK2_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim2_fsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM2_FSEL_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM2_FSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr2_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR2_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR2_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr2_tc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR2_TC_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR2_TC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch2_step(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH2_STEP_MASK),
                             (kal_uint32)(PMIC_ISINK_CH2_STEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath2_toff_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TOFF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TOFF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath2_ton_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TON_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath2_trf_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK2_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TRF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH2_TRF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch3_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH3_MODE_MASK),
                             (kal_uint32)(PMIC_ISINK_CH3_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink3_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK3_RSV1_MASK),
                             (kal_uint32)(PMIC_ISINK3_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim3_duty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM3_DUTY_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM3_DUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink3_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK3_RSV0_MASK),
                             (kal_uint32)(PMIC_ISINK3_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_dim3_fsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_DIM3_FSEL_MASK),
                             (kal_uint32)(PMIC_ISINK_DIM3_FSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr3_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR3_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR3_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_sfstr3_tc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_SFSTR3_TC_MASK),
                             (kal_uint32)(PMIC_ISINK_SFSTR3_TC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch3_step(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH3_STEP_MASK),
                             (kal_uint32)(PMIC_ISINK_CH3_STEP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath3_toff_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TOFF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TOFF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath3_ton_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TON_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TON_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_breath3_trf_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK3_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TRF_SEL_MASK),
                             (kal_uint32)(PMIC_ISINK_BREATH3_TRF_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isinks_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINKS_RSV_MASK),
                             (kal_uint32)(PMIC_RG_ISINKS_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink3_double_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK3_DOUBLE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK3_DOUBLE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink2_double_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK2_DOUBLE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK2_DOUBLE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink1_double_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK1_DOUBLE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK1_DOUBLE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_isink0_double_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ISINK0_DOUBLE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ISINK0_DOUBLE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_trim_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TRIM_SEL_MASK),
                             (kal_uint32)(PMIC_RG_TRIM_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_trim_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_ANA0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_TRIM_EN_MASK),
                             (kal_uint32)(PMIC_RG_TRIM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_ni_isink3_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ISINK_ANA1),
                           (&val),
                           (kal_uint32)(PMIC_NI_ISINK3_STATUS_MASK),
                           (kal_uint32)(PMIC_NI_ISINK3_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_isink2_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ISINK_ANA1),
                           (&val),
                           (kal_uint32)(PMIC_NI_ISINK2_STATUS_MASK),
                           (kal_uint32)(PMIC_NI_ISINK2_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_isink1_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ISINK_ANA1),
                           (&val),
                           (kal_uint32)(PMIC_NI_ISINK1_STATUS_MASK),
                           (kal_uint32)(PMIC_NI_ISINK1_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_ni_isink0_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ISINK_ANA1),
                           (&val),
                           (kal_uint32)(PMIC_NI_ISINK0_STATUS_MASK),
                           (kal_uint32)(PMIC_NI_ISINK0_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_isink_phase0_dly_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_PHASE_DLY),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_PHASE0_DLY_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_PHASE0_DLY_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_phase1_dly_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_PHASE_DLY),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_PHASE1_DLY_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_PHASE1_DLY_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_phase2_dly_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_PHASE_DLY),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_PHASE2_DLY_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_PHASE2_DLY_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_phase3_dly_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_PHASE_DLY),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_PHASE3_DLY_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_PHASE3_DLY_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_phase_dly_tc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_PHASE_DLY),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_PHASE_DLY_TC_MASK),
                             (kal_uint32)(PMIC_ISINK_PHASE_DLY_TC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch0_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH0_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CH0_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch1_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH1_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CH1_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch2_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH2_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CH2_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_ch3_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CH3_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CH3_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_chop0_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CHOP0_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CHOP0_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_chop1_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CHOP1_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CHOP1_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_chop2_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CHOP2_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CHOP2_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_isink_chop3_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ISINK_EN_CTRL),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ISINK_CHOP3_EN_MASK),
                             (kal_uint32)(PMIC_ISINK_CHOP3_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_analdorsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ANALDORSV1_MASK),
                             (kal_uint32)(PMIC_RG_ANALDORSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vtcxo_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VTCXO_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VTCXO_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vtcxo_lp_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VTCXO_LP_SET_MASK),
                             (kal_uint32)(PMIC_VTCXO_LP_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vtcxo_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON1),
                           (&val),
                           (kal_uint32)(PMIC_QI_VTCXO_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VTCXO_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vtcxo_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vtcxo_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_EN_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vtcxo_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VTCXO_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VTCXO_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vtcxo_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON1),
                           (&val),
                           (kal_uint32)(PMIC_QI_VTCXO_EN_MASK),
                           (kal_uint32)(PMIC_QI_VTCXO_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_va_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VA_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VA_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_va_lp_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VA_LP_SET_MASK),
                             (kal_uint32)(PMIC_VA_LP_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_va_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON2),
                           (&val),
                           (kal_uint32)(PMIC_QI_VA_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VA_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_va_sense_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_SENSE_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VA_SENSE_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VA_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_EN_MASK),
                             (kal_uint32)(PMIC_RG_VA_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_va_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON2),
                           (&val),
                           (kal_uint32)(PMIC_QI_VA_EN_MASK),
                           (kal_uint32)(PMIC_QI_VA_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_analdorsv2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ANALDORSV2_MASK),
                             (kal_uint32)(PMIC_RG_ANALDORSV2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VA_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vtcxo_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcama_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON5),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAMA_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCAMA_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_va_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON5),
                           (&val),
                           (kal_uint32)(PMIC_QI_VA_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VA_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vtcxo_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON5),
                           (&val),
                           (kal_uint32)(PMIC_QI_VTCXO_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VTCXO_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_analdorsv3(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ANALDORSV3_MASK),
                             (kal_uint32)(PMIC_RG_ANALDORSV3_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vtcxo_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vtcxo_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vtcxo_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VTCXO_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VTCXO_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VA_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VA_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VA_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_va_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VA_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VA_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_fbsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_FBSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_FBSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcama_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAMA_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCAMA_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcama_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMA_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMA_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_reserve_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RESERVE_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_RESERVE_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aldo_reserve(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ALDO_RESERVE_MASK),
                             (kal_uint32)(PMIC_RG_ALDO_RESERVE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn33_on_ctrl_bt(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN33_ON_CTRL_BT_MASK),
                             (kal_uint32)(PMIC_VCN33_ON_CTRL_BT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_en_bt(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_EN_BT_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_EN_BT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_en_wifi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_EN_WIFI_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_EN_WIFI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn33_on_ctrl_wifi(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN33_ON_CTRL_WIFI_MASK),
                             (kal_uint32)(PMIC_VCN33_ON_CTRL_WIFI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn33_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON17),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN33_EN_MASK),
                           (kal_uint32)(PMIC_QI_VCN33_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcn28_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn28_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN28_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN28_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn28_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN28_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCN28_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn28_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN28_EN_MASK),
                           (kal_uint32)(PMIC_QI_VCN28_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vcn28_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN28_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCN28_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn28_lp_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN28_LP_SET_MASK),
                             (kal_uint32)(PMIC_VCN28_LP_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn28_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON20),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN28_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCN28_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vcn28_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON20),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN28_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCN28_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vcn33_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN33_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCN33_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn33_lp_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN33_LP_SET_MASK),
                             (kal_uint32)(PMIC_VCN33_LP_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn33_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON21),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN33_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCN33_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcn33_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn33_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ANALDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN33_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN33_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn33_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ANALDO_CON21),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN33_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCN33_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vio28_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO28_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VIO28_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio28_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO28_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VIO28_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vio28_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON0),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO28_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VIO28_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vio28_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO28_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VIO28_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio28_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO28_EN_MASK),
                             (kal_uint32)(PMIC_VIO28_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vio28_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON0),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO28_EN_MASK),
                           (kal_uint32)(PMIC_QI_VIO28_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vusb_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VUSB_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VUSB_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vusb_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VUSB_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VUSB_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vusb_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON2),
                           (&val),
                           (kal_uint32)(PMIC_QI_VUSB_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VUSB_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vusb_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vusb_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_EN_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vusb_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON2),
                           (&val),
                           (kal_uint32)(PMIC_QI_VUSB_EN_MASK),
                           (kal_uint32)(PMIC_QI_VUSB_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vmc_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMC_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VMC_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vmc_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMC_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VMC_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vmc_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON3),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMC_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VMC_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vmc_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VMC_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_int_dis_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_INT_DIS_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VMC_INT_DIS_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vmc_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON3),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMC_EN_MASK),
                           (kal_uint32)(PMIC_QI_VMC_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vmch_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMCH_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VMCH_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vmch_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMCH_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VMCH_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vmch_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON5),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMCH_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VMCH_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vmch_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vmch_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON5),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMCH_EN_MASK),
                           (kal_uint32)(PMIC_QI_VMCH_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vemc_3v3_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VEMC_3V3_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VEMC_3V3_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vemc_3v3_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VEMC_3V3_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VEMC_3V3_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vemc_3v3_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON6),
                           (&val),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vemc_3v3_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_EN_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vemc_3v3_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON6),
                           (&val),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_EN_MASK),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_vgp1_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP1_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VGP1_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp1_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP1_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VGP1_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vgp1_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON7),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP1_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VGP1_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vgp1_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp2_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP2_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VGP2_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp2_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP2_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VGP2_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vgp2_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON8),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP2_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VGP2_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vgp2_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp3_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP3_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VGP3_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp3_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP3_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VGP3_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vgp3_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON9),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP3_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VGP3_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vgp3_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn_1v8_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN_1V8_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCN_1V8_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn_1v8_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN_1V8_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCN_1V8_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcn_1v8_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCN_1V8_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VCN_1V8_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn_1v8_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON11),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN_1V8_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCN_1V8_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcn_1v8_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcn_1v8_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON11),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN_1V8_EN_MASK),
                           (kal_uint32)(PMIC_QI_VCN_1V8_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_re_digldorsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RE_DIGLDORSV1_MASK),
                             (kal_uint32)(PMIC_RE_DIGLDORSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim1_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM1_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VSIM1_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim1_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM1_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VSIM1_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsim1_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON13),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSIM1_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VSIM1_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vsim1_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim2_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM2_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VSIM2_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim2_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM2_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VSIM2_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim2_ther_shdn_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM2_THER_SHDN_EN_MASK),
                             (kal_uint32)(PMIC_VSIM2_THER_SHDN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vsim2_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON14),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSIM2_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VSIM2_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vsim2_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrtc_force_on(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRTC_FORCE_ON_MASK),
                             (kal_uint32)(PMIC_RG_VRTC_FORCE_ON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vrtc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VRTC_EN_MASK),
                             (kal_uint32)(PMIC_VRTC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vrtc_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON15),
                           (&val),
                           (kal_uint32)(PMIC_QI_VRTC_EN_MASK),
                           (kal_uint32)(PMIC_QI_VRTC_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vemc_3v3_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMC_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vusb_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio28_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO28_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIO28_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrtc_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRTC_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VRTC_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vemc_3v3_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON18),
                           (&val),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VEMC_3V3_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vmch_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON18),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMCH_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VMCH_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vmc_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON18),
                           (&val),
                           (kal_uint32)(PMIC_QI_VMC_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VMC_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vusb_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON18),
                           (&val),
                           (kal_uint32)(PMIC_QI_VUSB_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VUSB_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vio28_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON18),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO28_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VIO28_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vsim2_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSIM2_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VSIM2_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vsim1_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VSIM1_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VSIM1_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vibr_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIBR_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VIBR_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vgp3_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP3_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VGP3_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vgp2_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP2_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VGP2_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vgp1_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON19),
                           (&val),
                           (kal_uint32)(PMIC_QI_VGP1_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VGP1_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_re_digldorsv2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RE_DIGLDORSV2_MASK),
                             (kal_uint32)(PMIC_RE_DIGLDORSV2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio28_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO28_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIO28_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio28_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO28_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VIO28_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio28_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO28_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VIO28_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vusb_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vusb_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vusb_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VUSB_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VUSB_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMC_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vmc_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMC_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VMC_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VMC_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VMC_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VMC_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_stb_sel_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_STB_SEL_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VMC_STB_SEL_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmc_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMC_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VMC_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vmch_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VMCH_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VMCH_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_db_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_DB_EN_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_DB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_stb_sel_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_STB_SEL_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_STB_SEL_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vmch_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VMCH_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VMCH_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vemc_3v3_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VEMC_3V3_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VEMC_3V3_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_dl_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_DL_EN_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_DL_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_db_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_DB_EN_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_DB_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_stb_sel_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STB_SEL_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_STB_SEL_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vemc_3v3_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VEMC_3V3_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp1_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON28),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP1_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VGP1_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp2_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON29),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP2_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VGP2_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON30),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON30),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON30),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON30),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vgp3_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON30),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VGP3_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VGP3_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_af_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON31),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_AF_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCAM_AF_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_af_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON31),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_AF_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VCAM_AF_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcam_af_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON31),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAM_AF_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCAM_AF_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcam_af_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON31),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON31),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_af_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_AF_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCAM_AF_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_af_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON32),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_re_digldorsv3(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON33),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RE_DIGLDORSV3_MASK),
                             (kal_uint32)(PMIC_RE_DIGLDORSV3_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON34),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON34),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON34),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON34),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim1_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON34),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM1_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM1_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON35),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON35),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON35),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON35),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsim2_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON35),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSIM2_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VSIM2_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vsysldo_reserve(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON36),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VSYSLDO_RESERVE_MASK),
                             (kal_uint32)(PMIC_RG_VSYSLDO_RESERVE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vibr_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON39),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIBR_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VIBR_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vibr_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON39),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIBR_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VIBR_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vibr_ther_shen_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON39),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIBR_THER_SHEN_EN_MASK),
                             (kal_uint32)(PMIC_VIBR_THER_SHEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vibr_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON39),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIBR_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VIBR_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vibr_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON39),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON39),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_stb_sel_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_STB_SEL_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_STB_SEL_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vibr_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON40),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIBR_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VIBR_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_digldo_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON41),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DIGLDO_RSV1_MASK),
                             (kal_uint32)(PMIC_DIGLDO_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_digldo_rsv0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON41),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_DIGLDO_RSV0_MASK),
                             (kal_uint32)(PMIC_DIGLDO_RSV0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ldo_ft(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON41),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LDO_FT_MASK),
                             (kal_uint32)(PMIC_RG_LDO_FT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcam_io_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAM_IO_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCAM_IO_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vcamd_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAMD_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCAMD_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vcn_1v8_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCN_1V8_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCN_1V8_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vio18_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO18_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VIO18_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vrf18_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VRF18_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VRF18_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vm_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VM_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VM_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_qi_vcam_af_oc_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON42),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAM_AF_OC_STATUS_MASK),
                           (kal_uint32)(PMIC_QI_VCAM_AF_OC_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcam_af_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_AF_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_AF_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_io_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_IO_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_IO_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcn_1v8_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCN_1V8_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCN_1V8_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio18_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VM_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_bist_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON43),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_BIST_EN_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_BIST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vibr_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIBR_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VIBR_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim2_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM2_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VSIM2_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vsim1_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VSIM1_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VSIM1_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp3_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP3_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VGP3_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp2_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP2_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VGP2_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vgp1_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON44),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VGP1_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VGP1_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vrf18_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON45),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VRF18_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VRF18_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vrf18_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON45),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VRF18_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VRF18_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vrf18_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON45),
                           (&val),
                           (kal_uint32)(PMIC_QI_VRF18_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VRF18_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vrf18_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON45),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON45),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_EN_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON46),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vrf18_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON46),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VRF18_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VRF18_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON46),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON46),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vrf18_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON46),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VRF18_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VRF18_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vm_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON47),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VM_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VM_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vm_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON47),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VM_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VM_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vm_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON47),
                           (&val),
                           (kal_uint32)(PMIC_QI_VM_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VM_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vm_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON47),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VM_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON47),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_EN_MASK),
                             (kal_uint32)(PMIC_RG_VM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vm_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON47),
                           (&val),
                           (kal_uint32)(PMIC_QI_VM_EN_MASK),
                           (kal_uint32)(PMIC_QI_VM_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vm_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VM_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_plcur_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_PLCUR_EN_MASK),
                             (kal_uint32)(PMIC_RG_VM_PLCUR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_plcur_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_PLCUR_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VM_PLCUR_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VM_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VM_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vm_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON48),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VM_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VM_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio18_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON49),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO18_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VIO18_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio18_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON49),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO18_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VIO18_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vio18_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON49),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO18_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VIO18_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vio18_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON49),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio18_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON49),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vio18_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON49),
                           (&val),
                           (kal_uint32)(PMIC_QI_VIO18_EN_MASK),
                           (kal_uint32)(PMIC_QI_VIO18_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vio18_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON50),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vio18_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON50),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VIO18_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VIO18_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio18_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON50),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio18_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON50),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vio18_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON50),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VIO18_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VIO18_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcamd_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON51),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAMD_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCAMD_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcamd_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON51),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAMD_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VCAMD_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcamd_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON51),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAMD_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCAMD_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcamd_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON51),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON51),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcamd_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON51),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAMD_EN_MASK),
                           (kal_uint32)(PMIC_QI_VCAMD_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcamd_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcamd_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAMD_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCAMD_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_vosel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_VOSEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_VOSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcamd_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON52),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAMD_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCAMD_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_io_lp_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON53),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_IO_LP_SEL_MASK),
                             (kal_uint32)(PMIC_VCAM_IO_LP_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_io_lp_mode_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON53),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_IO_LP_MODE_SET_MASK),
                             (kal_uint32)(PMIC_VCAM_IO_LP_MODE_SET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcam_io_mode(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON53),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAM_IO_MODE_MASK),
                           (kal_uint32)(PMIC_QI_VCAM_IO_MODE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcam_io_stbtd(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON53),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_STBTD_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_STBTD_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_io_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON53),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_qi_vcam_io_en(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(DIGLDO_CON53),
                           (&val),
                           (kal_uint32)(PMIC_QI_VCAM_IO_EN_MASK),
                           (kal_uint32)(PMIC_QI_VCAM_IO_EN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_vcam_io_ndis_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON54),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_NDIS_EN_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_NDIS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_vcam_io_on_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON54),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_VCAM_IO_ON_CTRL_MASK),
                             (kal_uint32)(PMIC_VCAM_IO_ON_CTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_io_ocfb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON54),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_OCFB_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_OCFB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_io_stb_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON54),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_STB_SEL_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_STB_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vcam_io_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(DIGLDO_CON54),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VCAM_IO_CAL_MASK),
                             (kal_uint32)(PMIC_RG_VCAM_IO_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_addr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_ADDR_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_ADDR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_prog(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_PROG_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_PROG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_EN_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_pkey(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_PKEY_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_PKEY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_rd_trig(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_RD_TRIG_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_RD_TRIG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_prog_src(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_PROG_SRC_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_PROG_SRC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_skip_efuse_out(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SKIP_EFUSE_OUT_MASK),
                             (kal_uint32)(PMIC_RG_SKIP_EFUSE_OUT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_rd_rdy_bypass(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_RD_RDY_BYPASS_MASK),
                             (kal_uint32)(PMIC_RG_RD_RDY_BYPASS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_efuse_rd_ack(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_CON6),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_RD_ACK_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_RD_ACK_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_busy(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_CON6),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_BUSY_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_BUSY_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_efuse_val_0_15(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_0_15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_0_15_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_0_15_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_16_31(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_16_31),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_16_31_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_16_31_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_32_47(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_32_47),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_32_47_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_32_47_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_48_63(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_48_63),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_48_63_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_48_63_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_64_79(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_64_79),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_64_79_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_64_79_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_80_95(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_80_95),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_80_95_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_80_95_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_96_111(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_96_111),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_96_111_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_96_111_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_112_127(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_112_127),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_112_127_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_112_127_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_128_143(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_128_143),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_128_143_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_128_143_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_144_159(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_144_159),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_144_159_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_144_159_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_160_175(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_160_175),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_160_175_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_160_175_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_efuse_val_176_191(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_VAL_176_191),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_176_191_MASK),
                             (kal_uint32)(PMIC_RG_EFUSE_VAL_176_191_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_efuse_dout_0_15(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_0_15),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_0_15_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_0_15_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_16_31(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_16_31),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_16_31_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_16_31_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_32_47(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_32_47),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_32_47_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_32_47_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_48_63(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_48_63),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_48_63_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_48_63_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_64_79(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_64_79),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_64_79_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_64_79_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_80_95(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_80_95),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_80_95_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_80_95_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_96_111(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_96_111),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_96_111_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_96_111_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_112_127(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_112_127),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_112_127_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_112_127_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_128_143(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_128_143),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_128_143_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_128_143_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_144_159(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_144_159),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_144_159_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_144_159_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_160_175(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_160_175),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_160_175_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_160_175_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_efuse_dout_176_191(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(EFUSE_DOUT_176_191),
                           (&val),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_176_191_MASK),
                           (kal_uint32)(PMIC_RG_EFUSE_DOUT_176_191_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_otp_pa(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OTP_PA_MASK),
                             (kal_uint32)(PMIC_RG_OTP_PA_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_otp_pdin(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OTP_PDIN_MASK),
                             (kal_uint32)(PMIC_RG_OTP_PDIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_otp_ptm(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(EFUSE_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OTP_PTM_MASK),
                             (kal_uint32)(PMIC_RG_OTP_PTM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_eosc32_opt(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_EOSC32_OPT_MASK),
                             (kal_uint32)(PMIC_MIX_EOSC32_OPT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_mix_xosc32_stp_cpdtb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(RTC_MIX_CON0),
                           (&val),
                           (kal_uint32)(PMIC_MIX_XOSC32_STP_CPDTB_MASK),
                           (kal_uint32)(PMIC_MIX_XOSC32_STP_CPDTB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_mix_xosc32_stp_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_PWDB_MASK),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_mix_xosc32_stp_lpdtb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(RTC_MIX_CON0),
                           (&val),
                           (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDTB_MASK),
                           (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDTB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_mix_xosc32_stp_lpden(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDEN_MASK),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_xosc32_stp_lpdrst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDRST_MASK),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_LPDRST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_xosc32_stp_cali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_CALI_MASK),
                             (kal_uint32)(PMIC_MIX_XOSC32_STP_CALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_stmp_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_STMP_MODE_MASK),
                             (kal_uint32)(PMIC_STMP_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_eosc32_stp_chop_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_EOSC32_STP_CHOP_EN_MASK),
                             (kal_uint32)(PMIC_MIX_EOSC32_STP_CHOP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_dcxo_stp_lvsh_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_DCXO_STP_LVSH_EN_MASK),
                             (kal_uint32)(PMIC_MIX_DCXO_STP_LVSH_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_pmu_stp_ddlo_vrtc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_PMU_STP_DDLO_VRTC_MASK),
                             (kal_uint32)(PMIC_MIX_PMU_STP_DDLO_VRTC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_pmu_stp_ddlo_vrtc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_PMU_STP_DDLO_VRTC_EN_MASK),
                             (kal_uint32)(PMIC_MIX_PMU_STP_DDLO_VRTC_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_rtc_stp_xosc32_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_RTC_STP_XOSC32_ENB_MASK),
                             (kal_uint32)(PMIC_MIX_RTC_STP_XOSC32_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_dcxo_stp_test_deglitch_mode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_DCXO_STP_TEST_DEGLITCH_MODE_MASK),
                             (kal_uint32)(PMIC_MIX_DCXO_STP_TEST_DEGLITCH_MODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_eosc32_stp_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_EOSC32_STP_RSV_MASK),
                             (kal_uint32)(PMIC_MIX_EOSC32_STP_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_eosc32_vct_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_EOSC32_VCT_EN_MASK),
                             (kal_uint32)(PMIC_MIX_EOSC32_VCT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_mix_stp_bbwakeup(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_STP_BBWAKEUP_MASK),
                             (kal_uint32)(PMIC_MIX_STP_BBWAKEUP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_mix_stp_rtc_ddlo(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(RTC_MIX_CON1),
                           (&val),
                           (kal_uint32)(PMIC_MIX_STP_RTC_DDLO_MASK),
                           (kal_uint32)(PMIC_MIX_STP_RTC_DDLO_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_mix_rtc_xosc32_enb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(RTC_MIX_CON1),
                           (&val),
                           (kal_uint32)(PMIC_MIX_RTC_XOSC32_ENB_MASK),
                           (kal_uint32)(PMIC_MIX_RTC_XOSC32_ENB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_mix_efuse_xosc32_enb_opt(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(RTC_MIX_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_MIX_EFUSE_XOSC32_ENB_OPT_MASK),
                             (kal_uint32)(PMIC_MIX_EFUSE_XOSC32_ENB_OPT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vcfg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VCFG_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VCFG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vupg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VUPG_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VUPG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vpwdb_pga(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VPWDB_PGA_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VPWDB_PGA_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vpwdb_adc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VPWDB_ADC_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VPWDB_ADC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vadc_denb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VADC_DENB_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VADC_DENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vadc_dvref_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VADC_DVREF_CAL_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VADC_DVREF_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vref24_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VREF24_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VREF24_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vcm14_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VCM14_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VCM14_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vcmsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VCMSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VCMSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_chs_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_CHS_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_CHS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audull_vcali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULL_VCALI_MASK),
                             (kal_uint32)(PMIC_RG_AUDULL_VCALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vcfg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VCFG_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VCFG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vupg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VUPG_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VUPG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vpwdb_pga(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VPWDB_PGA_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VPWDB_PGA_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vpwdb_adc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VPWDB_ADC_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VPWDB_ADC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vadc_denb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VADC_DENB_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VADC_DENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vadc_dvref_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VADC_DVREF_CAL_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VADC_DVREF_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vref24_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VREF24_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VREF24_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vcm14_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VCM14_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VCM14_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vcmsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VCMSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VCMSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_chs_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_CHS_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_CHS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audulr_vcali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDULR_VCALI_MASK),
                             (kal_uint32)(PMIC_RG_AUDULR_VCALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud_igbias_cali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD_IGBIAS_CALI_MASK),
                             (kal_uint32)(PMIC_RG_AUD_IGBIAS_CALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aud_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUD_RSV_MASK),
                             (kal_uint32)(PMIC_RG_AUD_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_amuter(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AMUTER_MASK),
                             (kal_uint32)(PMIC_RG_AMUTER_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_amutel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AMUTEL_MASK),
                             (kal_uint32)(PMIC_RG_AMUTEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adacl_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADACL_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_ADACL_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adacr_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADACR_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_ADACR_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_abias_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ABIAS_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_ABIAS_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aoutl_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AOUTL_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_AOUTL_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_aoutr_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AOUTR_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_AOUTR_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_acali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ACALI_MASK),
                             (kal_uint32)(PMIC_RG_ACALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_apgr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_APGR_MASK),
                             (kal_uint32)(PMIC_RG_APGR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_apgl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_APGL_MASK),
                             (kal_uint32)(PMIC_RG_APGL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_abuf_bias(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ABUF_BIAS_MASK),
                             (kal_uint32)(PMIC_RG_ABUF_BIAS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_abuf_inshort(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ABUF_INSHORT_MASK),
                             (kal_uint32)(PMIC_RG_ABUF_INSHORT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ahfmode(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AHFMODE_MASK),
                             (kal_uint32)(PMIC_RG_AHFMODE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adacck_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADACCK_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADACCK_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_dacref(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DACREF_MASK),
                             (kal_uint32)(PMIC_RG_DACREF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adepopx_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADEPOPX_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADEPOPX_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adepopx(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADEPOPX_MASK),
                             (kal_uint32)(PMIC_RG_ADEPOPX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_depop_vcm_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DEPOP_VCM_EN_MASK),
                             (kal_uint32)(PMIC_RG_DEPOP_VCM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_depop_vcmsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DEPOP_VCMSEL_MASK),
                             (kal_uint32)(PMIC_RG_DEPOP_VCMSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_depop_cursel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DEPOP_CURSEL_MASK),
                             (kal_uint32)(PMIC_RG_DEPOP_CURSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_chargeoption_depop(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CHARGEOPTION_DEPOP_MASK),
                             (kal_uint32)(PMIC_RG_CHARGEOPTION_DEPOP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_avcmgen_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AVCMGEN_EN_MASK),
                             (kal_uint32)(PMIC_RG_AVCMGEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auddl_vref24_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDDL_VREF24_EN_MASK),
                             (kal_uint32)(PMIC_RG_AUDDL_VREF24_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_abirsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ABIRSV_MASK),
                             (kal_uint32)(PMIC_RG_ABIRSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_float(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_FLOAT_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_FLOAT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vdpg(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VDPG_MASK),
                             (kal_uint32)(PMIC_RG_VDPG_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_bias(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_BIAS_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_BIAS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vdepop(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VDEPOP_MASK),
                             (kal_uint32)(PMIC_RG_VDEPOP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_v2spk(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_V2SPK_MASK),
                             (kal_uint32)(PMIC_RG_V2SPK_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_hsoutstbenh(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_HSOUTSTBENH_MASK),
                             (kal_uint32)(PMIC_RG_HSOUTSTBENH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_audtop_con8_rsv_0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_AUDTOP_CON8_RSV_0_MASK),
                             (kal_uint32)(PMIC_AUDTOP_CON8_RSV_0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_clksq_monen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_MONEN_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_MONEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auddigmicen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDDIGMICEN_MASK),
                             (kal_uint32)(PMIC_RG_AUDDIGMICEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audpwdbmicbias(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDPWDBMICBIAS_MASK),
                             (kal_uint32)(PMIC_RG_AUDPWDBMICBIAS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auddigmicpduty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDDIGMICPDUTY_MASK),
                             (kal_uint32)(PMIC_RG_AUDDIGMICPDUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auddigmicnduty(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDDIGMICNDUTY_MASK),
                             (kal_uint32)(PMIC_RG_AUDDIGMICNDUTY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auddigmicbias(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDDIGMICBIAS_MASK),
                             (kal_uint32)(PMIC_RG_AUDDIGMICBIAS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audmicbiasvref(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDMICBIASVREF_MASK),
                             (kal_uint32)(PMIC_RG_AUDMICBIASVREF_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audsparevmic(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDSPAREVMIC_MASK),
                             (kal_uint32)(PMIC_RG_AUDSPAREVMIC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbirx_zcd_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_EN_MASK),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbirx_zcd_cali(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_CALI_MASK),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_CALI_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbirx_zcd_hys_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUDTOP_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_HYS_ENB_MASK),
                             (kal_uint32)(PMIC_RG_VBIRX_ZCD_HYS_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_vbirx_zcd_status(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUDTOP_CON9),
                           (&val),
                           (kal_uint32)(PMIC_RG_VBIRX_ZCD_STATUS_MASK),
                           (kal_uint32)(PMIC_RG_VBIRX_ZCD_STATUS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_batsns(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC0),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATSNS_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATSNS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_batsns(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC0),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATSNS_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATSNS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_isense(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC1),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_ISENSE_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_ISENSE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_isense(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC1),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_ISENSE_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_ISENSE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_vcdt(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC2),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_VCDT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_VCDT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_vcdt(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC2),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_VCDT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_VCDT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_baton1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC3),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATON1_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATON1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_baton1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC3),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATON1_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATON1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_thr_sense1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC4),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_THR_SENSE1_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_THR_SENSE1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_thr_sense1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC4),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_THR_SENSE1_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_THR_SENSE1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_thr_sense2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC5),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_THR_SENSE2_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_THR_SENSE2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_thr_sense2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC5),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_THR_SENSE2_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_THR_SENSE2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_baton2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC6),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATON2_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_BATON2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_baton2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC6),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATON2_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_BATON2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_ch5(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC7),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_CH5_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_CH5_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_ch5(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC7),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_CH5_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_CH5_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_wakeup_pchr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC8),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_WAKEUP_PCHR_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_WAKEUP_PCHR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_wakeup_pchr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC8),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_WAKEUP_PCHR_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_WAKEUP_PCHR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_wakeup_swchr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC9),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_WAKEUP_SWCHR_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_WAKEUP_SWCHR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_wakeup_swchr(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC9),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_WAKEUP_SWCHR_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_WAKEUP_SWCHR_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_lbat(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC10),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_LBAT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_LBAT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_lbat(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC10),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_LBAT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_LBAT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_ch6(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC11),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_CH6_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_CH6_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_ch6(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC11),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_CH6_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_CH6_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_gps(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC12),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_GPS_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_GPS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_gps(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC13),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_GPS_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_GPS_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_gps_lsb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC14),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_GPS_LSB_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_GPS_LSB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_md(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC15),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_MD_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_MD_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_md_lsb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC16),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_MD_LSB_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_MD_LSB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_md(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC16),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_MD_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_MD_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_int(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC17),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_INT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_INT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_rdy_int(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC17),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_RDY_INT_MASK),
                           (kal_uint32)(PMIC_RG_ADC_RDY_INT_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_rsv1(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC18),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV1_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV1_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_rsv2(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC19),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV2_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV2_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_adc_out_rsv3(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_ADC20),
                           (&val),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV3_MASK),
                           (kal_uint32)(PMIC_RG_ADC_OUT_RSV3_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_sw_gain_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_RSV1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SW_GAIN_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_SW_GAIN_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_sw_offset_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_RSV2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SW_OFFSET_TRIM_MASK),
                             (kal_uint32)(PMIC_RG_SW_OFFSET_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_pwdb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_PWDB_MASK),
                             (kal_uint32)(PMIC_RG_ADC_PWDB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_pwdb_swctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_PWDB_SWCTRL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_PWDB_SWCTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_cali_rate(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CALI_RATE_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CALI_RATE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_cali_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CALI_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CALI_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_cali_force(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CALI_FORCE_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CALI_FORCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_autorst_range(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_AUTORST_RANGE_MASK),
                             (kal_uint32)(PMIC_RG_ADC_AUTORST_RANGE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_autorst_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_AUTORST_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_AUTORST_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_latch_edge(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_LATCH_EDGE_MASK),
                             (kal_uint32)(PMIC_RG_ADC_LATCH_EDGE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_filter_order(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_FILTER_ORDER_MASK),
                             (kal_uint32)(PMIC_RG_ADC_FILTER_ORDER_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_swctrl_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_SWCTRL_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_SWCTRL_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adcin_vsen_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adcin_vsen_mux_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_MUX_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADCIN_VSEN_MUX_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adcin_vbat_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADCIN_VBAT_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADCIN_VBAT_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adcin_chr_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADCIN_CHR_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADCIN_CHR_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_chsel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_CHSEL_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_CHSEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_debt_max(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_DEBT_MAX_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_DEBT_MAX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_debt_min(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_DEBT_MIN_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_DEBT_MIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_det_prd_15_0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_DET_PRD_15_0_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_DET_PRD_15_0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_det_prd_19_16(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_DET_PRD_19_16_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_DET_PRD_19_16_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_volt_max(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_VOLT_MAX_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_VOLT_MAX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_irq_en_max(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_IRQ_EN_MAX_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_IRQ_EN_MAX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_en_max(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_EN_MAX_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_EN_MAX_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_lbat_max_irq_b(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_CON5),
                           (&val),
                           (kal_uint32)(PMIC_RG_LBAT_MAX_IRQ_B_MASK),
                           (kal_uint32)(PMIC_RG_LBAT_MAX_IRQ_B_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_lbat_volt_min(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_VOLT_MIN_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_VOLT_MIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_irq_en_min(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_IRQ_EN_MIN_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_IRQ_EN_MIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_lbat_en_min(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_LBAT_EN_MIN_MASK),
                             (kal_uint32)(PMIC_RG_LBAT_EN_MIN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_rg_lbat_min_irq_b(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_CON6),
                           (&val),
                           (kal_uint32)(PMIC_RG_LBAT_MIN_IRQ_B_MASK),
                           (kal_uint32)(PMIC_RG_LBAT_MIN_IRQ_B_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_lbat_debounce_count_max(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_CON7),
                           (&val),
                           (kal_uint32)(PMIC_RG_LBAT_DEBOUNCE_COUNT_MAX_MASK),
                           (kal_uint32)(PMIC_RG_LBAT_DEBOUNCE_COUNT_MAX_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_rg_lbat_debounce_count_min(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(AUXADC_CON8),
                           (&val),
                           (kal_uint32)(PMIC_RG_LBAT_DEBOUNCE_COUNT_MIN_MASK),
                           (kal_uint32)(PMIC_RG_LBAT_DEBOUNCE_COUNT_MIN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_rg_data_reuse_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DATA_REUSE_SEL_MASK),
                             (kal_uint32)(PMIC_RG_DATA_REUSE_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_auxadc_bist_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUXADC_BIST_ENB_MASK),
                             (kal_uint32)(PMIC_RG_AUXADC_BIST_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_osr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OSR_MASK),
                             (kal_uint32)(PMIC_RG_OSR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_osr_gps(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_OSR_GPS_MASK),
                             (kal_uint32)(PMIC_RG_OSR_GPS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch7_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH7_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH7_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch6_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH6_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH6_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch5_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH5_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH5_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch4_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH4_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH4_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch3_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH3_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH3_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch2_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH2_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH2_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_ch0_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH0_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_CH0_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_calen(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_CALEN_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_CALEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_exten(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_EXTEN_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_EXTEN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_byp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_BYP_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_BYP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vbuf_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VBUF_EN_MASK),
                             (kal_uint32)(PMIC_RG_VBUF_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_source_lbat_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_SOURCE_LBAT_SEL_MASK),
                             (kal_uint32)(PMIC_RG_SOURCE_LBAT_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_gain_ch0_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH0_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH0_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_offset_ch0_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON13),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH0_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH0_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_gain_ch4_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON14),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH4_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH4_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_offset_ch4_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH4_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH4_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_gain_ch7_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH7_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_GAIN_CH7_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_efuse_offset_ch7_trim(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON17),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH7_TRIM_MASK),
                             (kal_uint32)(PMIC_EFUSE_OFFSET_CH7_TRIM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_ibias(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_IBIAS_MASK),
                             (kal_uint32)(PMIC_RG_ADC_IBIAS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_RST_MASK),
                             (kal_uint32)(PMIC_RG_ADC_RST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_lp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_LP_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_LP_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_input_short(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_INPUT_SHORT_MASK),
                             (kal_uint32)(PMIC_RG_ADC_INPUT_SHORT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_chopper_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CHOPPER_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CHOPPER_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vpwdb_adc(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VPWDB_ADC_MASK),
                             (kal_uint32)(PMIC_RG_VPWDB_ADC_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_vref18_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_VREF18_EN_MASK),
                             (kal_uint32)(PMIC_RG_VREF18_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_chs_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CHS_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CHS_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_dvref_cal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_DVREF_CAL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_DVREF_CAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_denb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_DENB_MASK),
                             (kal_uint32)(PMIC_RG_ADC_DENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_sleep_mode_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_SLEEP_MODE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_SLEEP_MODE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_gps_status(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_GPS_STATUS_MASK),
                             (kal_uint32)(PMIC_RG_ADC_GPS_STATUS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_md_status(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_MD_STATUS_MASK),
                             (kal_uint32)(PMIC_RG_ADC_MD_STATUS_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_test_mode_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TEST_MODE_EN_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TEST_MODE_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_test_out_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TEST_OUT_SEL_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TEST_OUT_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_deci_bypass_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DECI_BYPASS_EN_MASK),
                             (kal_uint32)(PMIC_RG_DECI_BYPASS_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_clk_aon(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_CLK_AON_MASK),
                             (kal_uint32)(PMIC_RG_ADC_CLK_AON_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_deci_force(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_DECI_FORCE_MASK),
                             (kal_uint32)(PMIC_RG_ADC_DECI_FORCE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_deci_gdly(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_DECI_GDLY_MASK),
                             (kal_uint32)(PMIC_RG_ADC_DECI_GDLY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_md_rqst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON20),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_MD_RQST_MASK),
                             (kal_uint32)(PMIC_RG_MD_RQST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_gps_rqst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON21),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_GPS_RQST_MASK),
                             (kal_uint32)(PMIC_RG_GPS_RQST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ap_rqst_list(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON22),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AP_RQST_LIST_MASK),
                             (kal_uint32)(PMIC_RG_AP_RQST_LIST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ap_rqst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON22),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AP_RQST_MASK),
                             (kal_uint32)(PMIC_RG_AP_RQST_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_ap_rqst_list_rsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON23),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AP_RQST_LIST_RSV_MASK),
                             (kal_uint32)(PMIC_RG_AP_RQST_LIST_RSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_out_trim_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_OUT_TRIM_ENB_MASK),
                             (kal_uint32)(PMIC_RG_ADC_OUT_TRIM_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_trim_comp(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_COMP_MASK),
                             (kal_uint32)(PMIC_RG_ADC_TRIM_COMP_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_2s_comp_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_2S_COMP_ENB_MASK),
                             (kal_uint32)(PMIC_RG_ADC_2S_COMP_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_cic_out_raw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CIC_OUT_RAW_MASK),
                             (kal_uint32)(PMIC_RG_CIC_OUT_RAW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_data_skip_enb(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DATA_SKIP_ENB_MASK),
                             (kal_uint32)(PMIC_RG_DATA_SKIP_ENB_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_data_skip_num(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON24),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_DATA_SKIP_NUM_MASK),
                             (kal_uint32)(PMIC_RG_DATA_SKIP_NUM_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_rev(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON25),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_REV_MASK),
                             (kal_uint32)(PMIC_RG_ADC_REV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_rsv1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON26),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_RSV1_MASK),
                             (kal_uint32)(PMIC_RG_ADC_RSV1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_adc_rsv2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(AUXADC_CON27),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_ADC_RSV2_MASK),
                             (kal_uint32)(PMIC_RG_ADC_RSV2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audaccdetvthcal(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDACCDETVTHCAL_MASK),
                             (kal_uint32)(PMIC_RG_AUDACCDETVTHCAL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audaccdetswctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDACCDETSWCTRL_MASK),
                             (kal_uint32)(PMIC_RG_AUDACCDETSWCTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audaccdettvdet(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDACCDETTVDET_MASK),
                             (kal_uint32)(PMIC_RG_AUDACCDETTVDET_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audaccdetvin1pulllow(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDACCDETVIN1PULLLOW_MASK),
                             (kal_uint32)(PMIC_RG_AUDACCDETVIN1PULLLOW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_audaccdetauxadcswctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_AUDACCDETAUXADCSWCTRL_MASK),
                             (kal_uint32)(PMIC_AUDACCDETAUXADCSWCTRL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_audaccdetauxadcswctrl_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_AUDACCDETAUXADCSWCTRL_SEL_MASK),
                             (kal_uint32)(PMIC_AUDACCDETAUXADCSWCTRL_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_rg_audaccdetrsv(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON0),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_AUDACCDETRSV_MASK),
                             (kal_uint32)(PMIC_RG_AUDACCDETRSV_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_EN_MASK),
                             (kal_uint32)(PMIC_ACCDET_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_seq_init(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON1),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_SEQ_INIT_MASK),
                             (kal_uint32)(PMIC_ACCDET_SEQ_INIT_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_cmp_pwm_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_CMP_PWM_EN_MASK),
                             (kal_uint32)(PMIC_ACCDET_CMP_PWM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_vth_pwm_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_VTH_PWM_EN_MASK),
                             (kal_uint32)(PMIC_ACCDET_VTH_PWM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_mbias_pwm_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_PWM_EN_MASK),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_PWM_EN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_cmp_pwm_idle(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_CMP_PWM_IDLE_MASK),
                             (kal_uint32)(PMIC_ACCDET_CMP_PWM_IDLE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_vth_pwm_idle(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_VTH_PWM_IDLE_MASK),
                             (kal_uint32)(PMIC_ACCDET_VTH_PWM_IDLE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_mbias_pwm_idle(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON2),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_PWM_IDLE_MASK),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_PWM_IDLE_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_pwm_width(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON3),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_PWM_WIDTH_MASK),
                             (kal_uint32)(PMIC_ACCDET_PWM_WIDTH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_pwm_thresh(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON4),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_PWM_THRESH_MASK),
                             (kal_uint32)(PMIC_ACCDET_PWM_THRESH_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_rise_delay(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_RISE_DELAY_MASK),
                             (kal_uint32)(PMIC_ACCDET_RISE_DELAY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_fall_delay(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON5),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_FALL_DELAY_MASK),
                             (kal_uint32)(PMIC_ACCDET_FALL_DELAY_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_debounce0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON6),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE0_MASK),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_debounce1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON7),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE1_MASK),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_debounce2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON8),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE2_MASK),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_debounce3(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON9),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE3_MASK),
                             (kal_uint32)(PMIC_ACCDET_DEBOUNCE3_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_ival_cur_in(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IVAL_CUR_IN_MASK),
                             (kal_uint32)(PMIC_ACCDET_IVAL_CUR_IN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_ival_sam_in(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IVAL_SAM_IN_MASK),
                             (kal_uint32)(PMIC_ACCDET_IVAL_SAM_IN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_ival_mem_in(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IVAL_MEM_IN_MASK),
                             (kal_uint32)(PMIC_ACCDET_IVAL_MEM_IN_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_ival_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON10),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IVAL_SEL_MASK),
                             (kal_uint32)(PMIC_ACCDET_IVAL_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_accdet_irq(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON11),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_IRQ_MASK),
                           (kal_uint32)(PMIC_ACCDET_IRQ_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_accdet_irq_clr(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON11),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IRQ_CLR_MASK),
                             (kal_uint32)(PMIC_ACCDET_IRQ_CLR_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE0_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE1_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode2(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE2_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE2_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode3(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE3_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE3_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode4(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE4_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE4_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_test_mode5(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE5_MASK),
                             (kal_uint32)(PMIC_ACCDET_TEST_MODE5_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_pwm_sel(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_PWM_SEL_MASK),
                             (kal_uint32)(PMIC_ACCDET_PWM_SEL_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_in_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_IN_SW_MASK),
                             (kal_uint32)(PMIC_ACCDET_IN_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_cmp_en_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_CMP_EN_SW_MASK),
                             (kal_uint32)(PMIC_ACCDET_CMP_EN_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_vth_en_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_VTH_EN_SW_MASK),
                             (kal_uint32)(PMIC_ACCDET_VTH_EN_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_mbias_en_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_EN_SW_MASK),
                             (kal_uint32)(PMIC_ACCDET_MBIAS_EN_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_pwm_en_sw(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON12),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_PWM_EN_SW_MASK),
                             (kal_uint32)(PMIC_ACCDET_PWM_EN_SW_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

kal_uint32 upmu_get_accdet_in(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_IN_MASK),
                           (kal_uint32)(PMIC_ACCDET_IN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_cur_in(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_CUR_IN_MASK),
                           (kal_uint32)(PMIC_ACCDET_CUR_IN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_sam_in(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_SAM_IN_MASK),
                           (kal_uint32)(PMIC_ACCDET_SAM_IN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_mem_in(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_MEM_IN_MASK),
                           (kal_uint32)(PMIC_ACCDET_MEM_IN_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_state(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_STATE_MASK),
                           (kal_uint32)(PMIC_ACCDET_STATE_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_mbias_clk(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_MBIAS_CLK_MASK),
                           (kal_uint32)(PMIC_ACCDET_MBIAS_CLK_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_vth_clk(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_VTH_CLK_MASK),
                           (kal_uint32)(PMIC_ACCDET_VTH_CLK_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_cmp_clk(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_CMP_CLK_MASK),
                           (kal_uint32)(PMIC_ACCDET_CMP_CLK_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_da_audaccdetauxadcswctrl(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON13),
                           (&val),
                           (kal_uint32)(PMIC_DA_AUDACCDETAUXADCSWCTRL_MASK),
                           (kal_uint32)(PMIC_DA_AUDACCDETAUXADCSWCTRL_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

kal_uint32 upmu_get_accdet_cur_deb(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(ACCDET_CON14),
                           (&val),
                           (kal_uint32)(PMIC_ACCDET_CUR_DEB_MASK),
                           (kal_uint32)(PMIC_ACCDET_CUR_DEB_SHIFT)
	                       );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);

  return val;
}

void upmu_set_accdet_rsv_con0(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON15),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_RSV_CON0_MASK),
                             (kal_uint32)(PMIC_ACCDET_RSV_CON0_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

void upmu_set_accdet_rsv_con1(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(ACCDET_CON16),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_ACCDET_RSV_CON1_MASK),
                             (kal_uint32)(PMIC_ACCDET_RSV_CON1_SHIFT)
	                         );
  pmic_unlock();
  if(ret!=0) printf("%d", ret);
}

