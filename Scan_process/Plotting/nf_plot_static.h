#ifndef NF_PLOT_STATIC_H
#define NF_PLOT_STATIC_H

#include <QObject>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include "Scan_process/Plotting/nf_plot_vna.h"
#include "Scan_process/Plotting/nf_plot_sa.h"


class NF_plot_static
{

public:
    NF_plot_static();

private:
    NF_plot *static_plot{};
};

#endif // NF_PLOT_STATIC_H
