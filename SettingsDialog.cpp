class SettingsDialog : public QDialog {
public:
    SettingsDialog(Widget *parent, QSettings *settings) : QDialog(parent), m_parent(parent), m_settings(settings) {
        setWindowTitle("App Settings");
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // Remove the "?" button
        setFixedSize(250, 200);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // --- Opacity Section ---
        layout->addWidget(new QLabel("Window Opacity:"));
        QSlider *opacitySlider = new QSlider(Qt::Horizontal, this);
        opacitySlider->setRange(20, 100); // 20% to 100%
        opacitySlider->setValue(m_parent->windowOpacity() * 100);
        layout->addWidget(opacitySlider);

        connect(opacitySlider, &QSlider::valueChanged, this, [this](int value) {
            m_parent->setWindowOpacity(value / 100.0);
            m_settings->setValue("opacity", value / 100.0);
        });

        // --- Startup Section ---
        QCheckBox *startupCheck = new QCheckBox("Start with Windows", this);

        // Check current Registry state
        QSettings bootSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        startupCheck->setChecked(bootSettings.contains("SAPAssistant"));

        connect(startupCheck, &QCheckBox::toggled, m_parent, &Widget::toggleAutoStart);
        layout->addWidget(startupCheck);

        // --- Close Button ---
        QPushButton *closeBtn = new QPushButton("Done", this);
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(closeBtn);
    }

private:
    Widget *m_parent;
    QSettings *m_settings;
};