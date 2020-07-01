#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QImage>
#include <QtGui>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::AboutDialog) {
    QLocale myLoc;
    ui->setupUi(this);
    ui->label->setText(qApp->applicationName()+". Version: " + qApp->applicationVersion());
    QString compiler;
    ui->label_os->setText(
                QString("OS: ")
                .append(QSysInfo::prettyProductName())
                .append(" / Product version: ")
                .append(QSysInfo::kernelType())
                .append(" ")
                .append(QSysInfo::productVersion()
                .append(" / Kernel version: ")
                .append(QSysInfo::kernelVersion())
                ));
#ifdef __clang_version__
    compiler.append(QString().asprintf("Compiler: clang %s",  __clang_version__));
#elif defined __GNUC__ && defined __VERSION__
    compiler.append(QString().asprintf("Compiler: gcc %s", __VERSION__));
#elif defined _MSC_VER
    compiler.append("Compiler: Visual Studio");
#if _MSC_VER >= 1920
    compiler.append(" 2019 / MSVC++ 16.").append(QString().asprintf("%d",((_MSC_VER % 100) - 20)));
#elif _MSC_VER > 1911
    compiler.append(" 2017 / MSVC++ 15.").append(QString().asprintf("%d",((_MSC_VER % 100) - 7)));
#elif _MSC_VER == 1911
    compiler.append(" 2017 / MSVC++ 15.3");
#elif _MSC_VER == 1910
    compiler.append(" 2017 / MSVC++ 15.0");
#elif _MSC_VER == 1900
    compiler.append(" 2015 / MSVC++ 14.0");
#elif _MSC_VER == 1800
    compiler.append(" 2013 / MSVC++ 12.0");
#elif _MSC_VER == 1700
    compiler.append(" 2012 / MSVC++ 11.0");
#elif _MSC_VER == 1600
    compiler.append(" 2010 / MSVC++ 10.0");
#elif  _MSC_VER == 1500
    compiler.append(" 2008 / MSVC++ 9.0");
#elif  _MSC_VER == 1400
    compiler.append(" 2005 / MSVC++ 8.0");
#elif  _MSC_VER == 1310
    compiler.append(" .NET 2003 / MSVC++ 7.1");
#elif  _MSC_VER == 1300
    compiler.append(" .NET 2002 / MSVC++ 7.0");
#elif  _MSC_VER == 1300
    compiler.append(" .NET 2002 / MSVC++ 7.0");
#else
    compiler.append(", unrecognised version");
#endif
    compiler.append(QString().asprintf(" (_MSC_VER=%d)", (int)_MSC_VER));
#endif

    ui->compiler->setText(compiler);
    ui->buttonBox->clear();
    setWindowTitle("About " + qApp->applicationName());
    QImage img1(":/images/Built_with_Qt_RGB_logo.png");
    ui->qt->setPixmap(QPixmap::fromImage(img1));
    QObject::connect(ui->qt, SIGNAL(customContextMenuRequested(QPoint)), qApp, SLOT(aboutQt()));
    continueButton = new QPushButton(tr("&Ok"));
    ui->buttonBox->addButton(continueButton, QDialogButtonBox::ActionRole);
    ui->buttonBox->connect(continueButton, SIGNAL(clicked()), this,
                           SLOT(on_pushButton_clicked()));
    setWindowFlags(windowFlags() & (~Qt::WindowMinMaxButtonsHint)& (~Qt::WindowContextHelpButtonHint));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_pushButton_clicked()
{
    this->close();
}


