/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef LITTLENAVMAP_MAINWINDOW_H
#define LITTLENAVMAP_MAINWINDOW_H

#include "common/maptypes.h"
#include "fs/fspaths.h"

#include <QMainWindow>
#include <QUrl>
#include <marble/MarbleGlobal.h>

class SearchController;
class RouteController;
class QComboBox;
class QLabel;
class Search;
class DatabaseManager;
class WeatherReporter;
class ConnectClient;
class ProfileWidget;
class InfoController;

namespace Marble {
class LegendWidget;
class MarbleAboutDialog;
class ElevationModel;
}

namespace atools {
namespace gui {
class Dialog;
class ErrorHandler;
class HelpHandler;
class FileHistoryHandler;
}

}

namespace Ui {
class MainWindow;
}

class MapWidget;
class MapQuery;
class InfoQuery;

class MainWindow :
  public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  virtual ~MainWindow();

  Ui::MainWindow *getUi() const
  {
    return ui;
  }

  MapWidget *getMapWidget() const
  {
    return mapWidget;
  }

  RouteController *getRouteController() const
  {
    return routeController;
  }

  const Marble::ElevationModel *getElevationModel();

  WeatherReporter *getWeatherReporter() const
  {
    return weatherReporter;
  }

  ConnectClient *getConnectClient() const
  {
    return connectClient;
  }

  void updateWindowTitle();

  void setShownMapObjectsMessageText(const QString& text = QString(), const QString& tooltipText = QString());
  void setStatusMessage(const QString& message);

signals:
  /* Emitted when window is shown the first time */
  void windowShown();

private:
  void connectAllSlots();
  void mainWindowShown();

  void writeSettings();
  void readSettings();
  void updateActionStates();
  void setupUi();

  void createNavMap();
  void options();
  void preDatabaseLoad();
  void postDatabaseLoad(atools::fs::FsPaths::SimulatorType type);

  void updateHistActions(int minIndex, int curIndex, int maxIndex);

  void updateMapShowFeatures();

  void increaseMapDetail();
  void decreaseMapDetail();
  void defaultMapDetail();
  void setMapDetail(int factor);
  void selectionChanged(const Search *source, int selected, int visible, int total);
  void routeSelectionChanged(int selected, int total);

  void routeNew();
  void routeOpen();
  void routeOpenRecent(const QString& routeFile);
  bool routeSave();
  bool routeSaveAs();
  void routeCenter();
  void renderStatusChanged(Marble::RenderStatus status);
  void resultTruncated(maptypes::MapObjectTypes type, int truncatedTo);
  bool routeCheckForChanges();
  bool routeValidate();
  void loadNavmapLegend();
  void showNavmapLegend();
  void resetMessages();
  void showDatabaseFiles();

  void kmlOpenRecent(const QString& kmlFile);
  void kmlOpen();
  void kmlClear();

  void legendAnchorClicked(const QUrl& url);

  void changeMapProjection(int index);
  void changeMapTheme(int index);

  /* Work on the close event that also catches clicking the close button
   * in the window frame */
  virtual void closeEvent(QCloseEvent *event) override;

  /* Emit a signal windowShown after first appearance */
  virtual void showEvent(QShowEvent *event) override;

  QString mainWindowTitle;
  SearchController *searchController = nullptr;
  RouteController *routeController = nullptr;
  atools::gui::FileHistoryHandler *routeFileHistory = nullptr, *kmlFileHistory = nullptr;
  QUrl legendUrl;

  QComboBox *mapThemeComboBox = nullptr, *mapProjectionComboBox = nullptr;
  int mapDetailFactor;

  Ui::MainWindow *ui;
  MapWidget *mapWidget = nullptr;
  ProfileWidget *profileWidget = nullptr;
  QLabel *mapDistanceLabel, *mapPosLabel, *renderStatusLabel, *detailLabel, *messageLabel;
  QStringList statusMessages;
  bool hasDatabaseLoadStatus = false;

  Marble::LegendWidget *legendWidget = nullptr;
  Marble::MarbleAboutDialog *marbleAbout = nullptr;
  atools::gui::Dialog *dialog = nullptr;
  atools::gui::ErrorHandler *errorHandler = nullptr;
  atools::gui::HelpHandler *helpHandler = nullptr;
  DatabaseManager *databaseManager = nullptr;
  WeatherReporter *weatherReporter = nullptr;
  ConnectClient *connectClient = nullptr;
  InfoController *infoController = nullptr;

  MapQuery *mapQuery = nullptr;
  InfoQuery *infoQuery = nullptr;
  bool firstStart = true, firstApplicationStart = false;

};

#endif // LITTLENAVMAP_MAINWINDOW_H
