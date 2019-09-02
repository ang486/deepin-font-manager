
#include "views/dfontmgrmainwindow.h"
#include "dfinstallnormalwindow.h"
#include "dsplitlistwidget.h"
#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"
#include "views/dfdeletedialog.h"
#include "views/dfontinfodialog.h"
#include "views/dfquickinstallwindow.h"

#include <QHBoxLayout>

#include <DApplication>
#include <DFileDialog>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DLog>
#include <DMenu>
#include <DSearchEdit>
#include <DSlider>
#include <DSplitter>
#include <DTitlebar>
#include <DWidgetUtil>

class DFontMgrMainWindowPrivate
{
public:
    DFontMgrMainWindowPrivate(DFontMgrMainWindow *q)
        : settingsQsPtr(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                                      QSettings::IniFormat))
        , q_ptr(q)
    {
    }

    //~DFontMgrMainWindowPrivate() {}

    DFrame *titleFrame {nullptr};

    DLabel *logoLabel {nullptr};

    DFrame *toolbar {nullptr};
    DIconButton *addFontButton {nullptr};
    DSearchEdit *searchFontEdit {nullptr};

    DFrame *fontShowArea {nullptr};

    DFrame *stateBar {nullptr};
    DLineEdit *textInputEdit {nullptr};
    DSlider *fontScaleSlider {nullptr};
    DLabel *fontSizeLabel {nullptr};

    QSplitter *mainWndSpliter {nullptr};
    DFrame *leftBarHolder {nullptr};
    DFrame *rightViewHolder {nullptr};

    // Menu
    DMenu *toolBarMenu {nullptr};
    DMenu *rightKeyMenu {nullptr};

    DSplitListWidget *leftSiderBar {nullptr};

    QScopedPointer<QSettings> settingsQsPtr;
    DFontMgrMainWindow *q_ptr;
    Q_DECLARE_PUBLIC(DFontMgrMainWindow)
};

DFontMgrMainWindow::DFontMgrMainWindow(bool isQuickMode, QWidget *parent)
    : DMainWindow(parent)
    , m_isQuickMode(isQuickMode)
    , m_fontManager(DFontManager::instance())
    , m_quickInstallWnd(new DFQuickInstallWindow())
    , d_ptr(new DFontMgrMainWindowPrivate(this))
{
    // setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint));

    initUI();
    initConnections();
}

DFontMgrMainWindow::~DFontMgrMainWindow() {}

void DFontMgrMainWindow::initData()
{
    D_D(DFontMgrMainWindow);

    // Not use now
    Q_UNUSED(d);
}
void DFontMgrMainWindow::initUI()
{
    setWindowRadius(18);  // debug
    initTileBar();
    initRightKeyMenu();
    initMainVeiws();
}
void DFontMgrMainWindow::initConnections()
{
    D_D(DFontMgrMainWindow);

    // Add Font button event
    QObject::connect(d->addFontButton, &DIconButton::clicked, this,
                     &DFontMgrMainWindow::handleAddFontEvent);

    QObject::connect(this, &DFontMgrMainWindow::fileSelected, this,
                     [this](const QStringList &files) { this->installFont(files); });
    // Menu event
    QObject::connect(d->toolBarMenu, &QMenu::triggered, this, &DFontMgrMainWindow::handleMenuEvent);

    // Right Key menu
    QObject::connect(d->rightKeyMenu, &QMenu::triggered, this,
                     &DFontMgrMainWindow::handleMenuEvent);
    // Initialize rigth menu it state
    QObject::connect(d->rightKeyMenu, &QMenu::aboutToShow, this, [=]() {
        DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

        // Disable delete menu for system font
        QAction *delAction = DFontMenuManager::getInstance()->getActionByMenuAction(
            DFontMenuManager::M_DeleteFont, DFontMenuManager::MenuType::RightKeyMenu);

        // Disable delete menu on system font
        if (nullptr != delAction && currItemData.pFontInfo->isSystemFont) {
            delAction->setDisabled(true);
        } else {
            delAction->setDisabled(false);
        }
    });

    // State bar event
    QObject::connect(d->fontScaleSlider, &DSlider::valueChanged, this, [this, d](int value) {
        QString fontSizeText;
        fontSizeText.sprintf(FMT_FONT_SIZE, value);
        d->fontSizeLabel->setText(fontSizeText);

        onFontSizeChanged(value);
    });

    // Quick install mode handle
    QObject::connect(this, &DFontMgrMainWindow::quickModeInstall, this,
                     [this](const QStringList &files) {
                         connect(m_quickInstallWnd.get(), &DFQuickInstallWindow::quickInstall, this,
                                 [this, files]() { this->installFont(files); });
                         m_quickInstallWnd.get()->setWindowModality(Qt::WindowModal);
                         m_quickInstallWnd->onFileSelected(files);
                         m_quickInstallWnd.get()->show();
                         Dtk::Widget::moveToCenter(m_quickInstallWnd.get());
                     });

    // Search text changed
    QObject::connect(d->searchFontEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onSearchTextChanged(const QString &)));

    QObject::connect(d->textInputEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onPreviewTextChanged(const QString &)));

    QObject::connect(d->leftSiderBar, SIGNAL(onListWidgetItemClicked(int)), this,
                     SLOT(onLeftSiderBarItemClicked(int)));

    QObject::connect(m_fontManager, SIGNAL(uninstallFontFinished(const QModelIndex &)), this, SLOT(onFontUninstallFinished(const QModelIndex &)));

}

void DFontMgrMainWindow::initTileBar()
{
    D_D(DFontMgrMainWindow);

    initTileFrame();

    d->toolBarMenu = DFontMenuManager::getInstance()->createToolBarSettingsMenu();

    bool isDXcbPlatform = true;

    if (isDXcbPlatform) {
        // d->toolbar->getSettingsButton()->hide();
        titlebar()->setMenu(d->toolBarMenu);
        titlebar()->setContentsMargins(0, 0, 0, 0);

        titlebar()->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
        titlebar()->setCustomWidget(d->titleFrame, false);
    }
}

void DFontMgrMainWindow::initTileFrame()
{
    D_D(DFontMgrMainWindow);

    d->logoLabel = new DLabel(this);
    d->logoLabel->setObjectName("LogoLabel");
    d->logoLabel->setFixedSize(QSize(32, 32));
    d->logoLabel->setFocusPolicy(Qt::NoFocus);
    d->logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->logoLabel->setPixmap(QPixmap(":/images/deepin-font-manager.svg"));

    d->titleFrame = new DFrame(this);
    d->titleFrame->setObjectName("TitleBar");
    d->titleFrame->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
    // d->titleFrame->setStyleSheet("background: yellow");  // debug
    d->titleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Add Font
    d->addFontButton = new DIconButton(DStyle::StandardPixmap::SP_IncreaseElement, this);
    d->addFontButton->setFixedSize(QSize(38, 38));
    d->addFontButton->setFlat(false);
    d->addFontButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    // Search font
    d->searchFontEdit = new DSearchEdit();
    // d->searchFontEdit->setFixedSize(QSize(FTM_SEARCH_BAR_W, FTM_SEARCH_BAR_H));
    d->searchFontEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->searchFontEdit->setPlaceHolder(QString("搜索"));

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);

    titleLayout->addSpacing(7);
    titleLayout->addWidget(d->logoLabel);
    titleLayout->addSpacing(14);
    titleLayout->addWidget(d->addFontButton);
    titleLayout->addSpacing(142);
    titleLayout->addWidget(d->searchFontEdit, 1);
    titleLayout->addSpacing(60);

    d->titleFrame->setLayout(titleLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->logoLabel->setStyleSheet("background: silver");
    d->addFontButton->setStyleSheet("background: silver");
    d->titleFrame->setStyleSheet("background: black");
    d->searchFontEdit->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initMainVeiws()
{
    D_D(DFontMgrMainWindow);

    d->mainWndSpliter = new QSplitter(Qt::Horizontal, this);
    // For Debug
    // d->mainWndSpliter->setStyleSheet("QSplitter::handle { background-color: red }");

    initLeftSideBar();
    initRightFontView();

    setCentralWidget(d->mainWndSpliter);
}

void DFontMgrMainWindow::initLeftSideBar()
{
    D_D(DFontMgrMainWindow);

    d->leftBarHolder = new DFrame(d->mainWndSpliter);
    d->leftBarHolder->setObjectName("leftMainLayoutHolder");
    d->leftBarHolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d->leftBarHolder->setFixedWidth(FTM_LEFT_SIDE_BAR_WITHD);
    d->leftBarHolder->setContentsMargins(10, 0, 10, 0);
    // d->leftBarHolder->setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout *leftMainLayout = new QVBoxLayout();
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(0);

    // ToDo:
    //    Need use the custom QListView replace QListWidget
    d->leftSiderBar = new DSplitListWidget(this);
    // leftSiderBar->setAttribute(Qt::WA_TranslucentBackground, true);
    d->leftSiderBar->setFrameShape(DFrame::NoFrame);
    d->leftSiderBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftMainLayout->addWidget(d->leftSiderBar);
    d->leftBarHolder->setLayout(leftMainLayout);

    d->leftSiderBar->setFocus();

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->leftBarHolder->setStyleSheet("background: blue");
    d->leftSiderBar->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initRightFontView()
{
    Q_D(DFontMgrMainWindow);

    d->rightViewHolder = new DFrame(d->mainWndSpliter);
    d->rightViewHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightViewHolder->setObjectName("rightMainLayoutHolder");

    QVBoxLayout *rightMainLayout = new QVBoxLayout();
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(0);

    d->fontShowArea = new DFrame(this);
    d->fontShowArea->setFrameShape(DFrame::NoFrame);
    d->fontShowArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initFontPreviewListView(d->fontShowArea);

    // initialize state bar
    initStateBar();

    rightMainLayout->addWidget(d->fontShowArea);
    rightMainLayout->addWidget(d->stateBar);

    d->rightViewHolder->setLayout(rightMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->fontShowArea->setStyleSheet("background: blue");
    m_fontPreviewListView->setStyleSheet("background: green");
    d->rightViewHolder->setStyleSheet("background: red");
#endif
}

//初始化字体预览ListView
void DFontMgrMainWindow::initFontPreviewListView(DFrame *parent)
{
    Q_D(DFontMgrMainWindow);

    QVBoxLayout *listViewVBoxLayout = new QVBoxLayout();
    listViewVBoxLayout->setMargin(0);
    listViewVBoxLayout->setContentsMargins(0, 0, 0, 0);
    listViewVBoxLayout->setSpacing(0);
    parent->setLayout(listViewVBoxLayout);

    m_fontPreviewListView = new DFontPreviewListView;
    m_fontPreviewListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_fontPreviewListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_fontPreviewListView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_fontPreviewListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontPreviewListView->setRightContextMenu(d->rightKeyMenu);

    listViewVBoxLayout->addWidget(m_fontPreviewListView);
}

void DFontMgrMainWindow::initStateBar()
{
    Q_D(DFontMgrMainWindow);

    QHBoxLayout *stateBarLayout = new QHBoxLayout();
    stateBarLayout->setContentsMargins(0, 0, 0, 0);
    stateBarLayout->setSpacing(0);

    d->stateBar = new DFrame(this);
    d->stateBar->setFrameShape(DFrame::NoFrame);
    d->stateBar->setFixedHeight(FTM_SBAR_HEIGHT);
    d->stateBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->textInputEdit = new DLineEdit(this);
    d->textInputEdit->setFixedHeight(FTM_SBAR_TXT_EDIT_H);
    d->textInputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->textInputEdit->setClearButtonEnabled(true);
    d->textInputEdit->setPlaceholderText("输入文本内容进行预览");

    d->fontScaleSlider = new DSlider(Qt::Orientation::Horizontal, this);
    d->fontScaleSlider->setFixedSize(FTM_SBAR_SLIDER_W, FTM_SBAR_SLIDER_H);
    // d->fontScaleSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->fontScaleSlider->setTracking(true);
    d->fontScaleSlider->setTickPosition(QSlider::NoTicks);
    d->fontScaleSlider->setRange(MIN_FONT_SIZE, MAX_FONT_SIZE);
    //设置初始显示字体大小
    d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);

    d->fontSizeLabel = new DLabel(this);
    d->fontSizeLabel->setFixedSize(FTM_SBAR_FSIZE_LABEL_W, FTM_SBAR_FSIZE_LABEL_H);
    d->fontSizeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    // d->fontSizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Init the default font size
    QString defaultFontSize;
    defaultFontSize.sprintf(FMT_FONT_SIZE, DEFAULT_FONT_SIZE);
    d->fontSizeLabel->setText(defaultFontSize);

    stateBarLayout->setAlignment(Qt::AlignVCenter | Qt::AlignVCenter);
    stateBarLayout->addSpacing(10);
    stateBarLayout->addWidget(d->textInputEdit);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontScaleSlider, 0, Qt::AlignVCenter);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontSizeLabel);
    stateBarLayout->addSpacing(20);

    d->stateBar->setLayout(stateBarLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->stateBar->setStyleSheet("background: green");
    d->textInputEdit->setStyleSheet("background: blue");
    d->fontScaleSlider->setStyleSheet("background: yellow");
    d->fontSizeLabel->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::handleAddFontEvent()
{
    Q_D(DFontMgrMainWindow);

    DFileDialog dialog;
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setNameFilter(Utils::suffixList());

    QString historyDir = d->settingsQsPtr->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    d->settingsQsPtr->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    Q_EMIT fileSelected(dialog.selectedFiles());
}

void DFontMgrMainWindow::handleMenuEvent(QAction *action)
{
    if (action->data().isValid()) {
        bool ok = false;
        int type = action->data().toInt(&ok);

        if (ok) {
            DFontMenuManager::MenuAction actionId = static_cast<DFontMenuManager::MenuAction>(type);

            // Add menu handler code here
            switch (actionId) {
            case DFontMenuManager::MenuAction::M_AddFont: {
                handleAddFontEvent();
            } break;
            case DFontMenuManager::MenuAction::M_ThemeDark:
                switchAppTheme(Theme::Dark);
                break;
            case DFontMenuManager::MenuAction::M_ThemeLight:
                switchAppTheme(Theme::Light);
                break;
            case DFontMenuManager::MenuAction::M_ThemeFollowSystem:
                switchAppTheme(Theme::FollowSystem);
                break;
            case DFontMenuManager::MenuAction::M_FontInfo: {
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                DFontInfoDialog fontInfoDlg(&currItemData);
                fontInfoDlg.exec();
            } break;
            case DFontMenuManager::MenuAction::M_DeleteFont: {
                DFDeleteDialog confirmDelDlg;
                connect(&confirmDelDlg, &DFDeleteDialog::accepted, this, [this]() {
                    // Add Delete font code Here
                    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                    qDebug() << "Confirm delete:" << currItemData.pFontInfo->filePath
                             << " is system font:" << currItemData.pFontInfo->isSystemFont;

                    QModelIndex currModelIndex = m_fontPreviewListView->currModelIndex();
                    QString uninstallFilePath = currItemData.pFontInfo->filePath;
                    m_fontManager->setType(DFontManager::UnInstall);
                    m_fontManager->setUnInstallFile(uninstallFilePath, currModelIndex);
                    m_fontManager->start();
                });

                confirmDelDlg.exec();

            } break;
            case DFontMenuManager::MenuAction::M_EnableOrDisable: {

                QModelIndex modelIndex = m_fontPreviewListView->currModelIndex();
                emit m_fontPreviewListView->onClickEnableButton(modelIndex);
            } break;
            case DFontMenuManager::MenuAction::M_Faverator: {

                QModelIndex modelIndex = m_fontPreviewListView->currModelIndex();
                emit m_fontPreviewListView->onClickCollectionButton(modelIndex);
            } break;
            case DFontMenuManager::MenuAction::M_ShowFontPostion:
                showFontFilePostion();
                break;
            case DFontMenuManager::MenuAction::M_Help: {
                emit quickModeInstall(QStringList());
            } break;
            default:
                qDebug() << "handleMenuEvent->(id=" << actionId << ")";
            }
        }
    }
}

void DFontMgrMainWindow::switchAppTheme(int type)
{
    DApplication *app = qobject_cast<DApplication *>(qApp);

    if (app) {
        switch (type) {
        case Theme::Dark: {
            app->setTheme("dark");
        } break;
        case Theme::Light: {
            app->setTheme("light");
        } break;
        case Theme::FollowSystem: {
            // Not implementated
        } break;
        default:
            qDebug() << "Unknow Theme type = " << type;
        }
    }
}

void DFontMgrMainWindow::installFont(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files;
    DFInstallNormalWindow dfNormalInstalldlg(files, this);
    if (m_isQuickMode) {
        dfNormalInstalldlg.setSkipException(true);
    }

    //安装结束后刷新字体列表
    connect(&dfNormalInstalldlg, &DFInstallNormalWindow::finishFontInstall, this, &DFontMgrMainWindow::onFontInstallFinished);

    Dtk::Widget::moveToCenter(&dfNormalInstalldlg);
    dfNormalInstalldlg.exec();
}

void DFontMgrMainWindow::initRightKeyMenu()
{
    Q_D(DFontMgrMainWindow);

    d->rightKeyMenu = DFontMenuManager::getInstance()->createRightKeyMenu();
}

void DFontMgrMainWindow::setQuickInstallMode(bool isQuick)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " isQuickMode=" << isQuick;
#endif
    m_isQuickMode = isQuick;
}

void DFontMgrMainWindow::onSearchTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    QString strSearchFontName = currStr;
    qDebug() << strSearchFontName << endl;

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    //根据搜索框内容实时过滤列表
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterFontNamePattern(strSearchFontName);

    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    QString previewText = d->textInputEdit->text();
    onPreviewTextChanged(previewText);
}

void DFontMgrMainWindow::onPreviewTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    QString previewText = currStr;
    if (0 == currStr.length()) {
        previewText = FTM_DEFAULT_PREVIEW_TEXT;
    }

    QString strFontSize = d->fontSizeLabel->text();
    int iFontSize = strFontSize.remove("px").toInt();

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(previewText), Qt::UserRole + 1);
        filterModel->setData(modelIndex, QVariant(iFontSize), Qt::UserRole + 2);
    }
}

void DFontMgrMainWindow::onFontSizeChanged(int fontSize)
{
    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(fontSize), Qt::UserRole + 2);
    }
}

void DFontMgrMainWindow::showFontFilePostion()
{
    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

    if (nullptr != currItemData.pFontInfo) {
        QUrl url
            = QUrl::fromLocalFile(QFileInfo(currItemData.pFontInfo->filePath).dir().absolutePath());
        qDebug() << QUrl::fromLocalFile(currItemData.pFontInfo->filePath).toString();
        QUrlQuery query;
        query.addQueryItem("selectUrl",
                           QUrl::fromLocalFile(currItemData.pFontInfo->filePath).toString());
        url.setQuery(query);

        QProcess::startDetached(DEEPIN_FILE_MANAGE_NAME, QStringList(url.toString()));
    }
}

void DFontMgrMainWindow::onLeftSiderBarItemClicked(int index)
{
    Q_D(DFontMgrMainWindow);
    qDebug() << index << endl;

    DSplitListWidget *listWidget = d->leftSiderBar;
    QListWidgetItem *item = listWidget->item(index);
    QVariant varUserData = item->data(Qt::UserRole).value<QVariant>();
    DSplitListWidget::FontGroup filterGroup = qvariant_cast<DSplitListWidget::FontGroup>(varUserData);

    if (varUserData.toInt() < 0) {
        qDebug() << varUserData.toInt() << endl;
        return;
    }
    qDebug() << filterGroup << endl;

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterGroup(filterGroup);

    QString previewText = d->textInputEdit->text();
    onPreviewTextChanged(previewText);
}

void DFontMgrMainWindow::onFontInstallFinished()
{
    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(filterModel->sourceModel());

    m_fontPreviewListView->refreshFontListData(sourceModel);
}

void DFontMgrMainWindow::onFontUninstallFinished(const QModelIndex &uninstallIndex)
{
    qDebug() << "finished remove row:" << uninstallIndex.row() << endl;
    m_fontPreviewListView->removeRowAtIndex(uninstallIndex);
}
