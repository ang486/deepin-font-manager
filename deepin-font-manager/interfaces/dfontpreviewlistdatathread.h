#ifndef DFONTPREVIEWLISTDATATHREAD_H
#define DFONTPREVIEWLISTDATATHREAD_H

#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QMetaType>

/*************************************************************************
 <Class>         DFontPreviewListDataThread
 <Description>   description
 <Author>
 <Note>          null
*************************************************************************/

class QFileSystemWatcher;
class DFontPreviewListView;
class DFontPreviewListDataThread : public QObject
{
    Q_OBJECT
public:
    static DFontPreviewListDataThread *instance(DFontPreviewListView *view);
    static DFontPreviewListDataThread *instance();

    DFontPreviewListDataThread(DFontPreviewListView *view);
    virtual ~DFontPreviewListDataThread();

    //从fontconfig配置文件同步字体启用/禁用状态数据
    void syncFontEnableDisableStatusData(const QStringList &disableFontPathList);
    //刷新字体列表的数据
    void refreshFontListData(bool isStartup = false, const QStringList &list = QStringList());
    //从字体信息链表中删除需要删除的项
    void removeFontData(const DFontPreviewItemData &removeItemData);
    //将需要添加项的字体数据收集放人list中
    int insertFontItemData(const QString &filePath,
                           int index,
                           const QStringList &chineseFontPathList,
                           const QStringList &monoSpaceFontPathList,
                           bool isStartup = false, bool isEnabled = true);
    //获取当前列表所有项的数据链表
    QList<DFontPreviewItemData> getFontModelList();
    //获取需要新增的字体数据链表
    QList<DFontPreviewItemData> getDiffFontModelList() const;
    //给线程锁赋值
    void setMutex(QMutex *mutex);
    //更新itemDataList的itemData状态
    void updateItemStatus(int index, const DFontPreviewItemData &itemData);
    //更新字体信息中的fontid
    void updateFontId(const DFontPreviewItemData &itemData, int id);
    //通过DFontData信息获取DFontPreviewItemData
    inline static DFontPreviewItemData getFontData(const FontData &fontData)
    {
        DFontPreviewItemData itemdata;
        itemdata.fontData = fontData;
//        qDebug() << __FUNCTION__ << fontData.strFontName;
        int index = m_fontModelList.indexOf(itemdata);
        if (index > -1) {
//            qDebug() << __FUNCTION__ << fontData.strFontName << index;
            return m_fontModelList.at(index);
        } else {
            qDebug() << __FUNCTION__ << " not found " << fontData.getFontType() << fontData.strFontName << DFontPreviewListDataThread::instance()->getFontModelList().size();
        }
        return itemdata;
    }

signals:
    //发出删除字体文件请求
    void requestDeleted(QStringList &files);
    //发出添加字体文件请求
    void requestAdded(const QStringList &files, bool isFirstInstall = false);
    //请求删除字体文件
    void requestForceDeleteFiles(QStringList &files);
    //请求重装继续
    void requestBatchReInstallContinue();
    //请求批量移除文件监视器
    void requestRemoveFileWatchers(const QStringList &files);
    //添加特定文件夹的文件监视器
    void requestAutoDirWatchers();
    //发出导出字体的请求
    void requestExportFont(const QStringList &files);
    //导出字体后，请求提示信息
    void exportFontFinished(int count);

protected slots:
    //线程函数
    void doWork();
    //初始化系统文件监视器
    void initFileSystemWatcher();

public slots:
    //文件修改后触发函数
    void updateChangedFile(const QString &path);
    //文件夹修改后触发函数
    void updateChangedDir();
    //添加文件监视器
    void addPathWatcher(const QString &path);
    //移除特定路径的文件监视器
    void removePathWatcher(const QString &path);
    //文件删除响应函数
    void onFileDeleted(QStringList &files);
    //增加文件响应函数
    void onFileAdded(const QStringList &files);
    //强制删除文件
    void forceDeleteFiles(QStringList &files);
    //批量移除文件监视器
    void onRemoveFileWatchers(const QStringList &files);
    //添加特定文件夹的文件监视器
    void onAutoDirWatchers();
    //导出字体文件
    void onExportFont(const QStringList &fontList);

public:
    QStringList m_allFontPathList;
    QStringList m_chineseFontPathList;
    QStringList m_monoSpaceFontPathList;
    static QList<DFontPreviewItemData> m_fontModelList;
    QList<DFontPreviewItemData> m_delFontInfoList;

protected:
    QThread mThread;

    DFMDBManager *m_dbManager {nullptr};
    QList<DFontPreviewItemData> m_diffFontModelList;
    DFontPreviewListView *m_view;
    QFileSystemWatcher *m_fsWatcher;
    QMutex *m_mutex;
};

#endif // DFONTPREVIEWLISTDATATHREAD_H
