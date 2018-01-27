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

#include "dfontinstall.h"
#include <QProcess>

DFontInstall::DFontInstall(QObject *parent)
    : QThread(parent)
{

}

DFontInstall::~DFontInstall()
{
}

void DFontInstall::startInstall(const QStringList &list)
{
    if (!m_fileList.isEmpty()) {
        m_fileList.clear();
    }
    m_fileList.append(list);

    QThread::start();
}

void DFontInstall::run()
{
    QProcess *process = new QProcess;
    bool failed = false;

    connect(process, &QProcess::readyReadStandardOutput, this, [=] { emit installStarted(); });
    process->start("pkexec", QStringList() << "cp" << "-r" << "-v" << m_fileList << "/usr/share/fonts");
    process->waitForFinished(-1);
    failed |= process->exitCode();

    if (!failed) {
        process->execute("fc-cache");
        process->waitForFinished(-1);
        emit installFinished();
    }

    process->deleteLater();
}