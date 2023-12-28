#include <QVBoxLayout>

#include <fmt/format.h>

#include "RedditConnectionFrame.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace owl
{

constexpr auto BGCOLOR = "white";

constexpr auto MAIN_STYLE = R"x(
QFrame#RedditConnectionFrame
{{
    background-color: {bgcolor};
}}

QLabel#testLabel
{{
    border: 5px solid black;
}}
)x"sv;

RedditConnectionFrame::RedditConnectionFrame(const std::string& uuid, QWidget* parent)
    : ConnectionFrame(uuid, parent)
{
    this->setObjectName("RedditConnectionFrame");

    const auto FrameStyleSheet { fmt::format(MAIN_STYLE, fmt::arg("bgcolor", BGCOLOR)) };
    this->setStyleSheet(FrameStyleSheet.data());

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    _label = new QLabel(this);
    _label->setObjectName("testLabel");
    _label->setText("RedditConnectionFrame");
    _label->setAlignment(Qt::AlignCenter);    

    verticalLayout->addWidget(_label);
    this->setLayout(verticalLayout);
}

} // namespace owl
