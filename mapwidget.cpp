/** @file mapwidget.cpp
 * Soubor s tridou MapWidget dedici ze tridy QDockWidget,
 * implementuje dokovaci okno s mapou
 */
#include "mapwidget.h"
#include <QWebEngineProfile>
#include <QtWebChannel>

MapWidget::MapWidget(QWidget* parent)
    : QDockWidget(parent)
{
    setWindowTitle(tr("Map"));

    mapView = new QWebEngineView(parent);
    QWebEngineProfile* profile = new QWebEngineProfile(mapView);
    profile->setHttpUserAgent(QString("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/%1 Geotagging/%2").arg(qVersion()).arg(GIT_VERSION));
    QWebEnginePage* page = new QWebEnginePage(profile, mapView);
    mapView->setPage(page);
    page->load(QUrl("qrc:///leaflet.html"));
    markersVisible = 1;
    routesVisible = 1;
    joinSegmentsVisible = 0;
    reliefVisible = 0;
    loadIsFinished = 0;
    connect(mapView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    QWebChannel* channel = new QWebChannel(mapView->page());
    channel->registerObject("mapWidget", this);
    mapView->page()->setWebChannel(channel);

    setAcceptDrops(false);
    mapView->setAcceptDrops(false);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    newMarkerCandidade = 0;
    setWidgets();
}

void MapWidget::loadFinished(bool n)
{
    Q_UNUSED(n)

    loadIsFinished = 1;
    if (!scriptsToRun.isEmpty()) {
        foreach (QString script, scriptsToRun)
            mapView->page()->runJavaScript(script, [](const QVariant& result) { qDebug() << result.toString(); });
    }
    scriptsToRun.clear();
    disconnect(mapView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}

void MapWidget::retranslateUi()
{

    mapSelect->setItemText(0, tr("Cyclo-tourist"));
    mapSelect->setItemText(1, tr("Google Roadmap"));
    mapSelect->setItemText(2, tr("Google Terrain"));
    mapSelect->setItemText(3, tr("Google Satellite"));
    mapSelect->setItemText(4, tr("Google Hybrid"));
    mapSelect->setItemText(5, tr("OSM Mapnik"));
    mapSelect->setItemText(6, tr("Mapy.cz base"));
    mapSelect->setItemText(7, tr("Mapy.cz Orthophoto"));
    mapSelect->setItemText(8, tr("ArcGis Topographic"));
    mapSelect->setItemText(9, tr("ArcGis Satellite"));

    bCenter->setToolTip(tr("Center map to view all"));
    bMarkersVisibility->setToolTip(tr("Show all markers"));
    bRoutesVisibility->setToolTip(tr("Show routes"));
    bJoinSegments->setToolTip(tr("Join segments"));
    bRelief->setToolTip(tr("Show relief"));

    setWindowTitle(tr("Map"));
}
void MapWidget::changeRouteOpacity(int id, int value)
{
    QString scriptStr = QString("changeRouteOpacity(%1, %2);").arg(id).arg(value / 10.0);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::lineWidthChanged(int id, int value)
{
    QString scriptStr = QString("lineWidthChanged(%1, %2);").arg(id).arg(value);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::changeRouteColor(int id, QString color)
{
    QString scriptStr = QString("changeRouteColor(%1, \"%2\");").arg(id).arg(color);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::settingNewMarker(QCursor cursor, QList<int> idList)
{
    this->setCursor(cursor);
    QString ids = "[";
    int i;
    foreach (i, idList) {
        ids += QString("%1").arg(i) + ",";
    }
    ids.chop(1);
    ids += "]";

    QString scriptStr = QString("settingNewMarker(%1);").arg(ids);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::endSettingNewMarker(QCursor cursor)
{
    this->setCursor(cursor);
    QString scriptStr = "endSettingNewMarker();";
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::settingNewObjectMarker(QCursor cursor, QList<int> idList)
{
    this->setCursor(cursor);
    QString ids = "[";
    int i;
    foreach (i, idList) {
        ids += QString("%1").arg(i) + ",";
    }
    ids.chop(1);
    ids += "]";

    QString scriptStr = QString("settingNewObjectMarker(%1);").arg(ids);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::settingRemoveCameraMarker(QList<int> idList)
{
    QString ids = "[";
    foreach (int i, idList) {
        ids += QString("%1").arg(i) + ",";
    }
    ids.chop(1);
    ids += "]";

    QString scriptStr = QString("settingRemoveCameraMarker(%1);").arg(ids);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::settingRemoveObjectMarker(QList<int> idList)
{
    QString ids = "[";
    foreach (int i, idList) {
        ids += QString("%1").arg(i) + ",";
    }
    ids.chop(1);
    ids += "]";

    QString scriptStr = QString("settingRemoveObjectMarker(%1);").arg(ids);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::endSettingNewObjectMarker(QCursor cursor)
{
    this->setCursor(cursor);
    QString scriptStr = "endSettingNewMarker();";
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::newCameraMarkerAdded(int id, double lat, double lon, double ele)
{
    qDebug() << "newCameraMarkerAdded" << id << lat << lon << ele;

    emit settingNewMarkerFinished();
    emit setGpsInImage(id, lat, lon, ele);
}

void MapWidget::newObjectMarkerAdded(int id, double lat, double lon, double ele)
{
    qDebug() << "newObjectMarkerAdded" << id << lat << lon << ele;

    emit settingNewMarkerFinished();
    emit setObjectGpsInImage(id, lat, lon, ele);
}

void MapWidget::directionUpdated(int id, double direction, double angleOfView)
{
    if (direction < -360) {
        direction = qQNaN();
    } else if (direction < 0) {
        direction += 360.0;
    }
    qDebug() << "directionUpdated" << id << direction << angleOfView;
    emit settingNewMarkerFinished();
    emit setCameraDirectionInImage(id, direction, angleOfView);
}

void MapWidget::setWidgets()
{
    iconMarkerVisible = new QIcon(":/icons/markerShowR.png");
    iconRouteVisible = new QIcon(":/icons/chodecMapa.png");
    iconRelief = new QIcon(":/icons/relief.png");
    iconJoinSegments = new QIcon(":/icons/routeFull.png");

    QSize iconSize(35, 35);
    QGridLayout* gl = new QGridLayout;
    int wCount = 0;
    mapSelect = new QComboBox;

    mapSelect->addItem(tr("Cyclo-tourist"), "Cykloturist");
    mapSelect->addItem(tr("Google Roadmap"), "ROADMAP");
    mapSelect->addItem(tr("Google Terrain"), "TERRAIN");
    mapSelect->addItem(tr("Google Satellite"), "SATELLITE");
    mapSelect->addItem(tr("Google Hybrid"), "HYBRID");
    mapSelect->addItem(tr("OSM Mapnik"), "OSMMapnik");
    mapSelect->addItem(tr("Mapy.cz base"), "mapyCzBase");
    mapSelect->addItem(tr("Mapy.cz Orthophoto"), "mapyCzOphoto");
    mapSelect->addItem(tr("ArcGis Topographic"), "arcGisTopo");
    mapSelect->addItem(tr("ArcGis Satellite"), "arcGitSatellite");
    mapSelect->setCurrentIndex(5);
    mapSelect->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    mapSelect->setFixedHeight(iconSize.height());

    connect(mapSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMap(int)));

    gl->addWidget(mapSelect, 0, wCount);
    wCount++;

    bCenter = new QToolButton;
    bCenter->setMaximumSize(iconSize);
    bCenter->setIcon(QIcon(":/icons/terc.png"));
    bCenter->setIconSize(bCenter->size());
    bCenter->setToolTip(tr("Center map to view all"));
    connect(bCenter, SIGNAL(pressed()), this, SLOT(centerMap()));
    gl->addWidget(bCenter, 0, wCount);

    QPalette pal = bCenter->palette();
    pal.setColor(QPalette::Window, "#D0D0E7");
    bCenter->setPalette(pal);
    bCenter->setBackgroundRole(QPalette::Window);
    wCount++;

    bMarkersVisibility = new QToolButton;
    bMarkersVisibility->setMaximumSize(iconSize);
    bMarkersVisibility->setIcon(*iconMarkerVisible);
    bMarkersVisibility->setIconSize(bMarkersVisibility->size());
    bMarkersVisibility->setToolTip(tr("Show all markers"));
    cMarkersVisibility = new QCheckBox(bMarkersVisibility);
    cMarkersVisibility->setChecked(markersVisible);
    cMarkersVisibility->setMaximumSize(QSize(15, 20));

    bMarkersVisibility->setContentsMargins(QMargins(0, 0, 0, 0));
    connect(cMarkersVisibility, SIGNAL(clicked()), this, SLOT(setMarkersVisibility()));
    connect(bMarkersVisibility, SIGNAL(clicked()), this, SLOT(setMarkersVisibility()));
    connect(bMarkersVisibility, SIGNAL(clicked()), cMarkersVisibility, SLOT(toggle()));
    gl->addWidget(bMarkersVisibility, 0, wCount);
    wCount++;

    bRoutesVisibility = new QToolButton;
    bRoutesVisibility->setMaximumSize(iconSize);
    bRoutesVisibility->setIcon(*iconRouteVisible);
    bRoutesVisibility->setIconSize(bRoutesVisibility->size());
    bRoutesVisibility->setToolTip(tr("Show routes"));
    cRoutesVisibility = new QCheckBox(bRoutesVisibility);
    cRoutesVisibility->setChecked(routesVisible);
    cRoutesVisibility->setMaximumSize(QSize(15, 20));
    bRoutesVisibility->setContentsMargins(QMargins(0, 0, 0, 0));
    ;

    connect(cRoutesVisibility, SIGNAL(clicked()), this, SLOT(setRoutesVisibility()));
    connect(bRoutesVisibility, SIGNAL(clicked()), this, SLOT(setRoutesVisibility()));
    connect(bRoutesVisibility, SIGNAL(clicked()), cRoutesVisibility, SLOT(toggle()));
    gl->addWidget(bRoutesVisibility, 0, wCount);
    wCount++;

    bJoinSegments = new QToolButton;
    bJoinSegments->setMaximumSize(iconSize);
    bJoinSegments->setIcon(*iconJoinSegments);
    bJoinSegments->setIconSize(bJoinSegments->size());
    bJoinSegments->setToolTip(tr("Join segments"));

    cJoinSegments = new QCheckBox(bJoinSegments);
    cJoinSegments->setChecked(joinSegmentsVisible);
    cJoinSegments->setMaximumSize(QSize(15, 20));
    bJoinSegments->setContentsMargins(QMargins(0, 0, 0, 0));
    ;
    connect(cJoinSegments, SIGNAL(clicked()), this, SLOT(setJoinSegments()));
    connect(bJoinSegments, SIGNAL(clicked()), this, SLOT(setJoinSegments()));
    connect(bJoinSegments, SIGNAL(clicked()), cJoinSegments, SLOT(toggle()));

    gl->addWidget(bJoinSegments, 0, wCount);
    wCount++;

    bRelief = new QToolButton;
    bRelief->setMaximumSize(iconSize);
    bRelief->setIcon(*iconRelief);
    bRelief->setIconSize(bRelief->size());
    bRelief->setToolTip(tr("Show relief"));
    cRelief = new QCheckBox(bRelief);
    cRelief->setChecked(reliefVisible);
    cRelief->setMaximumSize(QSize(15, 20));
    bJoinSegments->setContentsMargins(QMargins(0, 0, 0, 0));
    ;
    connect(cRelief, SIGNAL(clicked()), this, SLOT(flipRelief()));
    connect(bRelief, SIGNAL(clicked()), this, SLOT(flipRelief()));
    connect(bRelief, SIGNAL(clicked()), cRelief, SLOT(toggle()));

    gl->addWidget(bRelief, 0, wCount);
    wCount++;

    gl->addWidget(mapView, 1, 0, 1, wCount + 1);
    gl->setContentsMargins(QMargins(0, 0, 0, 0));
    QWidget* w = new QWidget;
    w->setLayout(gl);
    setWidget(w);
}

void MapWidget::flipRelief()
{
    reliefVisible = !reliefVisible;
    QStringList scriptStr;
    scriptStr << QString("flipRelief(%1);").arg(reliefVisible);
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::setJoinSegments()
{
    joinSegmentsVisible = !joinSegmentsVisible;
    QStringList scriptStr;
    scriptStr << QString("setJoinSegments(%1);").arg(joinSegmentsVisible && routesVisible);
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::changeMap(int mapI)
{
    QString s = mapSelect->itemData(mapI).toString();
    QStringList scriptStr;
    scriptStr << QString("setMapType(\"%1\");").arg(s);
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::addObjectMarker(int id, double lat, double lon)
{
    QStringList scriptStr;
    scriptStr << QString("addObjectMarker(%1, %2, %3, %4); centerInBounds(1,0);")
                     .arg(id)
                     .arg(markersVisible)
                     .arg(QString::number(lat, 'f', 10))
                     .arg(QString::number(lon, 'f', 10));

    if (!loadIsFinished) {
        scriptsToRun << scriptStr.join("\n");
    } else {
        mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
    }
}

void MapWidget::setttingCameraMarkerDirection(QList<int> idList, double direction)
{

    QString ids = "[";
    int i;
    foreach (i, idList) {
        ids += QString("%1").arg(i) + ",";
    }
    ids.chop(1);
    ids += "]";

    QStringList scriptStr;
    scriptStr << QString("setNewCameraDirection(%1, %2, %3);")
                     .arg(ids)
                     .arg(markersVisible)
                     .arg(qIsNaN(direction) ? "NaN" : QString::number(direction, 'f', 10));
    //    qDebug() << scriptStr.join("\n");

    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::addCameraMarker(int id, double lat, double lon, double direction, double angleOfView)
{
    QStringList scriptStr;
    scriptStr << QString("addCameraMarker(%1, %2, %3, %4, %5, %6); centerInBounds(1,0);")
                     .arg(id)
                     .arg(markersVisible)
                     .arg(QString::number(lat, 'f', 10))
                     .arg(QString::number(lon, 'f', 10))
                     .arg(qIsNaN(direction) ? "NaN" : QString::number(direction, 'f', 10))
                     .arg(qIsNaN(angleOfView) ? "NaN" : QString::number(angleOfView, 'f', 10));

    qDebug() << scriptStr;

    if (!loadIsFinished) {
        scriptsToRun << scriptStr.join("\n");
    } else {
        mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
    }
}

void MapWidget::markerSelected(int id, bool isSelected)
{
    QStringList scriptStr;
    scriptStr << QString("markerOrObjectSelected(%1, %2, %3);").arg(id).arg(isSelected).arg(markersVisible);
    //    qDebug() << scriptStr;
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::markerClicked(int id)
{
    QStringList scriptStr;
    scriptStr << QString("markerOrObjectClicked(%1, %2);").arg(id).arg((QApplication::keyboardModifiers() & Qt::ControlModifier));

    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
    emit mClicked(id, 0, 1);
}

void MapWidget::markerDragged(int id)
{
    qDebug() << "markerDragged" << id;
    idDragged = id;
    QMessageBox* mb = new QMessageBox(QMessageBox::Question, tr("Change camera location"),
        tr("Change camera coordinates to the new position?"),
        QMessageBox::Yes | QMessageBox::No);

    QPalette pal = mb->palette();
    pal.setColor(QPalette::Window, "#D0D0E7");
    mb->setPalette(pal);
    int ret = mb->exec();
    if (ret == QMessageBox::Yes) {
        setNewGpsInImage();
    } else {
        setMarkerLastPosition();
    }
}

void MapWidget::objectSelected(int id, bool isSelected)
{
    QStringList scriptStr;
    scriptStr << QString("markerOrObjectSelected(%1, %2, %3)").arg(id).arg(isSelected).arg(markersVisible);
    qDebug() << scriptStr;
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::objectClicked(int id)
{
    QStringList scriptStr;
    scriptStr << QString("markerOrObjectClicked(%1, %2);").arg(id).arg((QApplication::keyboardModifiers() & Qt::ControlModifier));

    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
    //    emit mClicked(id, 0, 1); // FIXME
}

void MapWidget::objectDragged(int id)
{
    qDebug() << "objectDragged" << id;
    idDragged = id;
    QMessageBox* mb = new QMessageBox(QMessageBox::Question, tr("Change picture object location"),
        tr("Change picture object coordinates to the new position?"),
        QMessageBox::Yes | QMessageBox::No);

    QPalette pal = mb->palette();
    pal.setColor(QPalette::Window, "#D0D0E7");
    mb->setPalette(pal);
    int ret = mb->exec();
    if (ret == QMessageBox::Yes) {
        setNewObjectPositionIntoImage();
    } else {
        setObjectMarkerLastPosition();
    }
}

void MapWidget::setNewGpsInImage()
{
    qDebug() << "setNewGpsInImage" << idDragged;
    QString scriptStr = QString("setNewMarkerPosition(%1);").arg(idDragged);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });

    //    mapView->page()->mainFrame()->evaluateJavaScript( scriptStr).toList();
}

void MapWidget::setNewObjectPositionIntoImage()
{
    qDebug() << "setNewGpsInImage" << idDragged;
    QString scriptStr = QString("setNewObjectMarkerPosition(%1);").arg(idDragged);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });

    //    mapView->page()->mainFrame()->evaluateJavaScript( scriptStr).toList();
}

void MapWidget::setMarkerLastPosition()
{
    QString scriptStr = QString("setOldMarkerPosition(%1);").arg(idDragged);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::setObjectMarkerLastPosition()
{
    QString scriptStr = QString("setOldObjectMarkerPosition(%1);").arg(idDragged);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::addRoute(GpsRoute* route)
{

    for (int i = 0; i < route->segmentList.length(); i++) {
        QString points = "";
        GpsSegment* seg = route->segmentList.at(i);
        for (int j = 0; j < seg->length(); j++) {
            points.append(QString("[%1, %2],\n")
                              .arg(QString::number(seg->at(j)->latitude, 'f', 10))
                              .arg(QString::number(seg->at(j)->longitude, 'f', 10)));
        }
        QStringList scriptStr;
        scriptStr << "var routeCoordinatesList = ["
                  << points
                  << "];"
                  << QString("addRoute(routeCoordinatesList, %1, %2, \"%3\", 1);")
                         .arg(route->id)
                         .arg(routesVisible)
                         .arg(route->routeColor->name());

        if (i + 1 < route->segmentList.length()) { // pridani spojeni segmentu
            scriptStr << "var routeCoordinatesList2 = ["
                      << QString("[%1, %2],").arg(QString::number(route->segmentList.at(i)->last()->latitude, 'f', 10)).arg(QString::number(route->segmentList.at(i)->last()->longitude, 'f', 10))
                      << QString("[%1, %2]").arg(QString::number(route->segmentList.at(i + 1)->first()->latitude, 'f', 10)).arg(QString::number(route->segmentList.at(i + 1)->first()->longitude, 'f', 10))
                      << "];"
                      << QString("addRoute(routeCoordinatesList2, %1, %2, \"%3\",0);").arg(route->id).arg(routesVisible && joinSegmentsVisible).arg(route->routeColor->name());
        }

        qDebug() << scriptStr.join("\n");

        if (!loadIsFinished) {
            scriptsToRun << scriptStr.join("\n");
        } else {
            mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
        }
    }
}

void MapWidget::centerMap()
{
    QStringList scriptStr;
    scriptStr << "centerInBounds(1,1)";
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::setMarkersVisibility()
{
    QStringList scriptStr;
    markersVisible = !markersVisible;
    scriptStr << QString("setMarkersVisibility(%1);").arg(markersVisible);
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::setRoutesVisibility()
{
    routesVisible = !routesVisible;
    QStringList scriptStr;
    scriptStr << QString("setRoutesVisibility(%1);").arg(routesVisible)
              << QString("setJoinSegments(%1);").arg(joinSegmentsVisible && routesVisible);

    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::deleteMarker(int id)
{
    QStringList scriptStr;
    scriptStr << QString("deleteMarker(%1);").arg(id);
    mapView->page()->runJavaScript(scriptStr.join("\n"), [](const QVariant& result) { qDebug() << result.toString(); });
}

void MapWidget::keyPressEvent(QKeyEvent* event)
{
    emit processEvent(event);
}

void MapWidget::deleteRoute(int id)
{
    QString scriptStr = QString("deleteRoute(%1);").arg(id);
    mapView->page()->runJavaScript(scriptStr, [](const QVariant& result) { qDebug() << result.toString(); });
}
