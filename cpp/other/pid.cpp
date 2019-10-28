#include <lakeshore.h>

LS_PID_ZONE zone_table;

LS_PID_ZONE::LS_PID_ZONE()
{
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        P[i] = 50; I[i] = 20; D[i] = 0;
        range[i] = 0;
        upper_boundary[i] = 0;
        ramp_rate[i] = 1;
    }
}

int LS_PID_ZONE::id(const int i, const string str)
{
    if(str == "UPPER_BOUNDARY")
    {
        const int id_upper_bound[] = {ID_EDITCONTROL_UBOUNDARY_1, ID_EDITCONTROL_UBOUNDARY_2, ID_EDITCONTROL_UBOUNDARY_3,
                            ID_EDITCONTROL_UBOUNDARY_4, ID_EDITCONTROL_UBOUNDARY_5, ID_EDITCONTROL_UBOUNDARY_6,
                            ID_EDITCONTROL_UBOUNDARY_7, ID_EDITCONTROL_UBOUNDARY_8, ID_EDITCONTROL_UBOUNDARY_9,
                            ID_EDITCONTROL_UBOUNDARY_10};
        return id_upper_bound[i];
    }
    else if(str == "P")
    {
        const int id_p[] = {ID_EDITCONTROL_P_1, ID_EDITCONTROL_P_2, ID_EDITCONTROL_P_3, ID_EDITCONTROL_P_4,
                        ID_EDITCONTROL_P_5, ID_EDITCONTROL_P_6, ID_EDITCONTROL_P_7, ID_EDITCONTROL_P_8,
                        ID_EDITCONTROL_P_9, ID_EDITCONTROL_P_10};
        return id_p[i];
    }
    else if(str == "I")
    {
        const int id_i[] = {ID_EDITCONTROL_I_1, ID_EDITCONTROL_I_2, ID_EDITCONTROL_I_3, ID_EDITCONTROL_I_4,
                        ID_EDITCONTROL_I_5, ID_EDITCONTROL_I_6, ID_EDITCONTROL_I_7, ID_EDITCONTROL_I_8,
                        ID_EDITCONTROL_I_9, ID_EDITCONTROL_I_10};
        return id_i[i];
    }
    else if(str == "D")
    {
        const int id_d[] = {ID_EDITCONTROL_D_1, ID_EDITCONTROL_D_2, ID_EDITCONTROL_D_3, ID_EDITCONTROL_D_4,
                        ID_EDITCONTROL_D_5, ID_EDITCONTROL_D_6, ID_EDITCONTROL_D_7, ID_EDITCONTROL_D_8,
                        ID_EDITCONTROL_D_9, ID_EDITCONTROL_D_10};
        return id_d[i];
    }
    else if(str == "RANGE")
    {
        const int id_range[] = {ID_COMBO_RANGE_1, ID_COMBO_RANGE_2, ID_COMBO_RANGE_3, ID_COMBO_RANGE_4,
                            ID_COMBO_RANGE_5, ID_COMBO_RANGE_6, ID_COMBO_RANGE_7, ID_COMBO_RANGE_8,
                            ID_COMBO_RANGE_9, ID_COMBO_RANGE_10};
        return id_range[i];
    }
    else if(str == "RAMP_RATE")
    {
        const int id_ramp[] = {ID_EDITCONTROL_RAMP_RATE_10, ID_EDITCONTROL_RAMP_RATE_9, ID_EDITCONTROL_RAMP_RATE_8,
                        ID_EDITCONTROL_RAMP_RATE_7, ID_EDITCONTROL_RAMP_RATE_6, ID_EDITCONTROL_RAMP_RATE_5,
                        ID_EDITCONTROL_RAMP_RATE_4, ID_EDITCONTROL_RAMP_RATE_3, ID_EDITCONTROL_RAMP_RATE_2,
                        ID_EDITCONTROL_RAMP_RATE_1};
        return id_ramp[i];
    }
    return -1;
}

