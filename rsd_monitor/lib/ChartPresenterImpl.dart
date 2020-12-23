import 'package:rsd_monitor/RSDRepository.dart';
import 'ChartPresenter.dart';
import 'dao/NASEntry.dart';
import 'package:scidart/numdart.dart';
import 'package:scidart/scidart.dart';

class ChartPresenterImpl implements ChartPresenter {
  ChartPageView _view;
  ChartPresenterImpl(this._view) {
    _view.setPresenter(this);
  }
  RSDRepository _repository = new RSDRepository();
  DateTime start;
  DateTime end;
  int currentMode = 0;
  int entryCount = 1000;
  int filterMode = 0;
  List<NASEntry> entryList = List();
  @override
  void setStartTime(DateTime t) {
    start = t;
  }

  @override
  DateTime getStartTime() {
    return start;
  }

  @override
  void setFilterMode(int i) {
    filterMode = i;
  }

  @override
  void setEndTime(DateTime t) {
    end = t;
  }

  @override
  DateTime getEndTime() {
    return end;
  }

  @override
  bool setEntryCount(String str) {
    int n = 0;
    try {
      n = int.parse(str);
    } on FormatException {
      print('Format error!');
      return false;
    }
    entryCount = n;
    return true;
  }

  @override
  init() async {
    await _repository.init();
    start = DateTime.now();
    end = DateTime.now();
    entryList = new List<NASEntry>();
  }

  @override
  void setCurrentMode(int i) {
    currentMode = i;
  }

  @override
  int getCurrentMode() {
    return currentMode;
  }

  void loadSuccess(List<NASEntry> data) {
    List<double> avg = List();
    List<double> max = List();
    List<double> min = List();
    if (filterMode != 0) {
      for (NASEntry item in data) {
        avg.add(item.avg_amp);
        max.add(item.max_amp);
        min.add(item.min_amp);
      }
    }

    switch (filterMode) {
      case 0: //no filter
        break;
      case 1: //0. filter
        var b = Array([1.0, 2.0, 3.0]);
        var a = Array([1.0, 2.0, 3.0]);
        avg = lfilter(b, a, avg);
        max = lfilter(b, a, max);
        min = lfilter(b, a, min);
        break;
      case 2: //1. filter
        var b = Array([1.0]);
        var a = Array([1.0, 2.0, 3.0]);
        avg = lfilter(b, a, avg);
        max = lfilter(b, a, max);
        min = lfilter(b, a, min);
        break;
      case 3: //2. filter
        var b = Array([1.0, 2.0, 3.0]);
        var a = Array([1.0]);
        avg = lfilter(b, a, avg);
        max = lfilter(b, a, max);
        min = lfilter(b, a, min);
        break;
    }
    if (filterMode != 0) {
      for (int i = 0; i < data.length; i++) {
        data[i].avg_amp = avg[i];
        data[i].max_amp = max[i];
        data[i].min_amp = min[i];
      }
    }
    _view.onLoadSuccess(data);
  }

  @override
  loadData() {
    print('load data');
    if (currentMode == 0) {
      //entryCount
      _repository.loadDataByCount(entryCount).then((data) {
        loadSuccess(data);
      }).catchError((error) {
        print(error.toString());
        _view.onLoadError(error.toString());
      });
    } else {
      //timeQuery
      _repository.loadDataByDate(start, end).then((data) {
        loadSuccess(data);
      }).catchError((error) {
        print(error.toString());
        _view.onLoadError(error.toString());
      });
    }
  }
}
