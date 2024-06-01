#include <QVBoxLayout>

#include <fmt/format.h>

#include "ChatConnectionFrame.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace owl
{

// constexpr auto BGCOLOR = "white";

// constexpr auto MAIN_STYLE = R"x(
// QFrame#ChatConnectionFrame
// {{
//     background-color: {bgcolor};
// }}

// QLabel#testLabel
// {{
//     border: 5px solid black;
// }}
// )x"sv;

ChatConnectionFrame::ChatConnectionFrame(QWidget* parent)
    : ConnectionFrame("ChatConnectionUUID", parent)
{
    setFocusPolicy(Qt::TabFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/ChatWindow.qml"));

    //     SettingsObject settings;
    // _dtOptions.useDefault = settings.read("datetime.format").toString() == "default";
    // _dtOptions.usePretty = settings.read("datetime.date.pretty").toBool();
    // _dtOptions.dateFormat = settings.read("datetime.date.format").toString();
    // _dtOptions.timeFormat = settings.read("datetime.time.format").toString();


    // QQmlContext* root = rootContext();
    // root = this->rootContext();
    // root->setContextProperty("threadListPage", this);
    // root->setContextProperty("threadListModel", QVariant{});

    

    // const auto FrameStyleSheet { fmt::format(MAIN_STYLE, fmt::arg("bgcolor", BGCOLOR)) };
    // this->setStyleSheet(FrameStyleSheet.data());

    // QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    // _label = new QLabel(this);
    // _label->setObjectName("testLabel");
    // _label->setText("ChatConnectionFrame");
    // _label->setAlignment(Qt::AlignCenter);    

    // verticalLayout->addWidget(_label);
    // this->setLayout(verticalLayout);
}

} // namespace owl
