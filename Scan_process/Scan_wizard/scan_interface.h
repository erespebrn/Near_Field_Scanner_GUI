#ifndef SCAN_INTERFACE_H
#define SCAN_INTERFACE_H

#include <QObject>
#include <QDebug>
#include <QMessageBox>
#include "videothread.h"
#include "videothread.h"
#include "ui_scanner_gui.h"

#include "Scan_process/Plotting/nf_plot_sa.h"
#include "Scan_process/Plotting/nf_plot_vna.h"

#include "Scan_process/PCB_size/dut_size.h"
#include "Scan_process/Scan_wizard/scanarea.h"
#include "TCP_IP/Robot/robot.h"
#include "TCP_IP/RS_Instr/rs_instruments.h"
#include "TCP_IP/RS_Instr/rs_sa.h"
#include "TCP_IP/RS_Instr/rs_vna.h"

class Scan_interface : public QWidget
{
    Q_OBJECT
public:
    Scan_interface(QWidget *parent = nullptr, Robot *r = nullptr, VideoThread *v = nullptr, Ui::scanner_gui *mui = nullptr);
    ~Scan_interface();

    enum Scan_stop_reason{SCAN_ABORTED, SCAN_ERROR, SCAN_FINISHED};

    //Public methods
    RS_Instruments *create_rs_instrument();
    void            create_dut_sizes(void);
    void            measure_height(void);
    void            goto_takepic2_position(void);
    void            set_takepic2_calib(void);
    void            goto_takepic2_calib(void);
    void            goto_scan_origin(void);
    void            goto_home_pos(void);
    void            goto_pcb_ref_pos(void);
    void            callib_x_setEnabled(bool);
    void            callib_y_setEnabled(bool);
    void            assign_scan_field_components(bool x, bool y);
    void            event_logfile_init(const QString&);
    void            start_scan(void);
    void            stop_scan(Scan_stop_reason);

    //Inline methods
    inline DUT_size &get_pcb_size(void)                 { return *pcb;                        };

    inline void     disp_height_scan_point(bool al)     { emit display_scan_height_point(al); };
    inline void     disp_pcb_outline(bool al)           { emit display_pcb_outline(al);       };
    inline void     disp_pcb_ref_point(bool al)         { emit display_pcb_ref_point(al);     };
    inline void     disp_pcb_corner_point(bool al)      { emit display_pcb_corner_point(al);  };
    inline void     sa_connected(bool c)                { sa_connected_bool = c;              };
    inline void     vna_connected(bool c)               { vna_connected_bool = c;             };
    inline void     update_height_scan(double sh)       { robot->scan_heigh_update(sh);       };
    inline int      get_estimated_time(void)            { return robot->total_time;           };


    bool            ins_created{false};
    bool            scan_height_measured{false};

signals:

    void            display_scan_height_point(bool);
    void            display_pcb_outline(bool);
    void            display_pcb_ref_point(bool);
    void            display_pcb_corner_point(bool);


    void            height_measured_to_wizard(void);
    void            scan_finished_to_wizard(void);
    void            scan_error_to_wizard(void);
    void            restart_instr_detector(void);

private slots:
    //Slots
    void            processCroppedImage(QRect &rect);

    //Inline slots
    inline void     set_pcb_selected_flag(bool s)             { pcb_sel = s;                   };
    inline void     set_height_scan_selected_flag(bool s)     { height_scan_point_sel = s;     };
    inline void     set_pcb_ref_point_selected_flag(bool s)   { pcb_ref_point_sel = s;         };
    inline void     set_scan_area_selected_flag(bool s)       { scan_area_sel = s;             };
    inline void     set_pcb_corner_selected_flag(bool s)      { pcb_corner_point_sel = s;      };

    inline void     height_measured()           { scan_height_measured = true;      };

protected:
    Ui::scanner_gui *mw_ui;

    double step_size{};
    bool pcb_sel{false};
    bool height_scan_point_sel{false};
    bool pcb_corner_point_sel{false};
    bool pcb_ref_point_sel{false};
    bool scan_area_sel{false};

    VideoThread *video;
    scanArea *scan_area_preview{};
    QString dut_name;

private:
    //Parent widget and ui

    QWidget *par{};

    //RS Instruments
    RS_Instruments *ins{};
    bool sa_connected_bool{false};
    bool vna_connected_bool{false};

    //Nearfield plot tool
    NF_plot *nearfield_plot{};

    //Robot
    Robot *robot{};

    //PCB and scan area objects
    DUT_size *pcb{};
    DUT_size *scan_area{};

    //Videothread
    QImage croppedImg{};

    //Log file
    Event_log *log_file{};

    //Saving datapaths
    QString datapath = "C:/Users/Near-field scanner/Documents/Near_Field_Scanner_GUI/datastorage/scan_data/";
    QString current_scan_datapath{};
};

#endif // SCAN_INTERFACE_H
