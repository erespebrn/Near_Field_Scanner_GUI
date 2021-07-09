#ifndef NF_PLOT_H
#define NF_PLOT_H

#include <QWidget>
#include <memory>
#include <fstream>
#include <QVector>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include "qcustomplot.h"

namespace Ui {
class NF_plot;
}

class NF_plot : public QWidget
{
    Q_OBJECT

public:
    enum Current_scan_comp{YCOMP, XCOMP};

    explicit NF_plot(QWidget *parent = nullptr);
    ~NF_plot();


    inline void set_current_scan_comp(Current_scan_comp sc)    { c_scan_com = sc; };

    virtual void init_plot(const size_t x, const size_t y) = 0;
    virtual void add_data_point(void) = 0;
    virtual void static_plot_init(QString&, bool, bool) = 0;

    void assign_field_comp(bool, bool);
    void allow_exports(bool);
    void add_bg_image(QImage*);

    void static_data_vectors_init(void);
    void static_maxhold_plot(void);

    void display_first_freq(float, float);

    inline void set_scan_components(bool x, bool y)  { x_comp = x, y_comp = y; };
    inline void set_dut_name(QString &dut)           { dut_name = dut;         };

    void assign_data_tensors(std::vector<float>*, std::vector<float>*,
                             std::vector<std::vector<float>>*,
                             std::vector<std::vector<std::vector<float>>>*,
                             std::vector<std::vector<std::vector<float>>>*,
                             std::vector<float>*,
                             std::vector<std::vector<float>>*,
                             std::vector<std::vector<std::vector<float>>>*,
                             std::vector<std::vector<std::vector<float>>>*);

    void plot_maxhold();
    void maxhold_init(const size_t&);
signals:
    void plot_widget_closing(void);

private slots:

    virtual void on_interpolate_radio_clicked(bool checked) = 0;
    virtual void on_save_selected_button_clicked() = 0;
    virtual void graphClicked(QCPAbstractPlottable *plottable, int dataIndex) = 0;
    virtual void freq_labelDoubleClicked(QMouseEvent *event) = 0;
    virtual void on_testSlider_valueChanged(int) = 0;
    virtual void axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart) = 0;


    void onColorRangeChanged(QCPRange range);
    void closeEvent(QCloseEvent *event) override;
    void on_export_plots_button_clicked();
    void on_cancel_saving_button_clicked();
    void on_default_radioButton_clicked(bool checked);
    void on_custom_radioButton_clicked(bool checked);





protected:

    Ui::NF_plot *ui;
    QString dut_name{};
    size_t x_points = 100;
    size_t y_points = 100;

    std::vector<std::vector<std::vector<float>>> *scan_data_mag;
    std::vector<std::vector<float>> *temp2d_mag;
    std::vector<float> *mag;

    std::vector<std::vector<std::vector<float>>> *scan_data_phase;
    std::vector<std::vector<float>> *temp2d_phase;
    std::vector<float> *phase;

    std::vector<std::vector<std::vector<float>>> *scan_data_t_mag;
    std::vector<std::vector<std::vector<float>>> *scan_data_t_phase;
    std::vector<float> *freq;

    std::vector<std::vector<std::vector<float>>> st_scan_data_mag;
    std::vector<std::vector<float>> st_temp2d_mag;
    std::vector<float> st_mag;

    std::vector<std::vector<std::vector<float>>> st_scan_data_phase;
    std::vector<std::vector<float>> st_temp2d_phase;
    std::vector<float> st_phase;

    std::vector<std::vector<std::vector<float>>> st_scan_data_t_mag;
    std::vector<std::vector<std::vector<float>>> st_scan_data_t_phase;
    std::vector<float> st_freq;

    QImage *pcb_img;
    QImage pcb_img_cpy;
    Current_scan_comp c_scan_com{YCOMP};
    QString current_scan_datapath{};

    bool x_comp{false};
    bool y_comp{false};
    bool y_finished = false;
    bool m_selected{false};

    int min=10;
    int max=80;
    int d_range = 40;
    int fr = 0;
    size_t static_sp{};
    uint16_t step_size{0};
    bool multiple_export_time = false;
    void add_single_marker(int dIndex);

    QCPAxisRect *heatmap_x_rect_mag{};
    QCPAxisRect *heatmap_y_rect_mag{};
    QCPAxisRect *heatmap_x_rect_phase{};
    QCPAxisRect *heatmap_y_rect_phase{};

    QCPColorScale *colorScale_x_mag{};
    QCPColorScale *colorScale_y_mag{};
    QCPColorScale *colorScale_x_phase{};
    QCPColorScale *colorScale_y_phase{};

    QCPColorMap *colorMap_y_mag{};
    QCPColorMap *colorMap_x_mag{};
    QCPColorMap *colorMap_y_phase{};
    QCPColorMap *colorMap_x_phase{};

    QCPItemPixmap *image_pixmap_y_mag{};
    QCPItemPixmap *image_pixmap_x_mag{};
    QCPItemPixmap *image_pixmap_y_phase{};
    QCPItemPixmap *image_pixmap_x_phase{};

    QCPTextElement *fr_label{};
    QCPItemTracer *tracer;
    QCPGraph *dwPoints;

    QVector<QPoint> exp_points;

    double low_lim{}, high_lim{};
    QVector<double> maxh, x, y;

private:




};

#endif // NF_PLOT_H

