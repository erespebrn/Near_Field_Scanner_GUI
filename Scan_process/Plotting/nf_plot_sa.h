#ifndef NF_PLOT_SA_H
#define NF_PLOT_SA_H

#include <QObject>
#include "Scan_process/Plotting/nf_plot.h"

class NF_plot_sa : public NF_plot
{
    Q_OBJECT
public:
    NF_plot_sa();
    virtual void init_plot(const size_t x, const size_t y) override;
    virtual void add_data_point(void) override;
    virtual void static_plot_init(QString&, bool, bool) override;

    void static_data_read(QString&);

    void replot_heatmap();

public slots:
    void on_interpolate_radio_clicked(bool checked) override;
    virtual void on_save_selected_button_clicked() override;
    virtual void graphClicked(QCPAbstractPlottable *plottable, int dataIndex) override;
    virtual void freq_labelDoubleClicked(QMouseEvent *event) override;
    virtual void on_testSlider_valueChanged(int) override;
    virtual void axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart) override;
private:
};

#endif // NF_PLOT_SA_H
