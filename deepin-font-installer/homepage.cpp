/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homepage.h"
#include "utils.h"
#include <DSvgRenderer>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_tipsLabel(new QLabel(tr("Drag font file here"))),
      m_splitLine(new QLabel),
      m_chooseBtn(new DLinkButton(tr("Select file")))
{
    const auto ratio = devicePixelRatioF();
    QPixmap iconPixmap = DSvgRenderer::render(":/images/font_unload.svg", QSize(160, 160) * ratio);
    iconPixmap.setDevicePixelRatio(ratio);

    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(160, 160);
    iconLabel->setPixmap(iconPixmap);
    m_splitLine->setPixmap(QPixmap(":/images/split_line.svg"));

    m_tipsLabel->setStyleSheet("QLabel { color: #6a6a6a; }");

    m_layout->addSpacing(40);
    m_layout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    m_layout->addSpacing(20);
    m_layout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_splitLine, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_chooseBtn, 0, Qt::AlignHCenter);
    m_layout->addStretch();
    m_layout->setSpacing(0);

    connect(m_chooseBtn, &DLinkButton::clicked, this, &HomePage::onChooseBtnClicked);
}

HomePage::~HomePage()
{
}

void HomePage::onChooseBtnClicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(Utils::suffixList());

    if (dialog.exec() != QDialog::Accepted)
        return;

    emit fileSelected(dialog.selectedFiles());
}
