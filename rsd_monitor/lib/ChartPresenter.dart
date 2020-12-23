import 'Base.dart';
abstract class ChartPresenter implements IPresenter {
  //init
  void loadData();
  void setStartTime(DateTime t);
  DateTime getStartTime();
  void setEndTime(DateTime t);
  DateTime getEndTime();
  bool setEntryCount(String str);
  void setCurrentMode(int i);
  int getCurrentMode();
  void setFilterMode(int i);
}

abstract class ChartPageView implements IView<ChartPresenter> {

  void onLoadSuccess(data);
  void onLoadError(String msg);
}