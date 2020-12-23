import 'package:flutter/material.dart';
import 'package:charts_flutter/flutter.dart' as charts;
import 'ChartPresenterImpl.dart';
import 'ChartPresenter.dart';
import 'package:toggle_switch/toggle_switch.dart';
import 'dao/NASEntry.dart';

void main() {
  runApp(MyApp());
}

/*
var results = await pool.query('select * from users');
  results.forEach((row) {
    print('Name: ${row[0]}, email: ${row[1]}');
  });
*/
class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'RSD Monitor',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: MyHomePage(title: 'RSD Monitor'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);
  final String title;

  @override
  _MyHomePageState createState() {
    _MyHomePageState view = new _MyHomePageState();
    ChartPresenter presenter = new ChartPresenterImpl(view);
    presenter.init();
    return view;
  }
}

class _MyHomePageState extends State<MyHomePage> implements ChartPageView {
  int _counter = 0;

  bool animate;
  ChartPresenter _presenter;
  List<charts.Series<dynamic, DateTime>> seriesList;
  List<NASEntry> entryList = List();
  @override
  void initState() {
    // TODO: implement initState
    super.initState();
    animate = true;
    seriesList = new List<charts.Series<dynamic, DateTime>>();
    seriesList.add(new charts.Series<NASEntry, DateTime>(
        id: 'Average Amp',
        colorFn: (_, __) => charts.MaterialPalette.blue.shadeDefault.lighter,
        domainFn: (NASEntry record, _) => record.time_index,
        measureFn: (NASEntry record, _) => record.avg_amp,
        data: entryList));
    seriesList.add(new charts.Series<NASEntry, DateTime>(
        id: 'Max Amp',
        colorFn: (_, __) => charts.MaterialPalette.red.shadeDefault.lighter,
        domainFn: (NASEntry record, _) => record.time_index,
        measureFn: (NASEntry record, _) => record.max_amp,
        data: entryList));
    seriesList.add(new charts.Series<NASEntry, DateTime>(
        id: 'Min Amp',
        colorFn: (_, __) => charts.MaterialPalette.green.shadeDefault.lighter,
        domainFn: (NASEntry record, _) => record.time_index,
        measureFn: (NASEntry record, _) => record.min_amp,
        data: entryList));
    _presenter.loadData();
  }

  _selectStartDate(context) async {
    final DateTime picked = await showDatePicker(
      context: context,
      initialDate: _presenter.getStartTime(), // Refer step 1
      firstDate: DateTime(2000),
      lastDate: DateTime(2025), 
    );
    if (picked != null && picked != _presenter.getStartTime())
      setState(() {
        _presenter.setStartTime(picked);
        if (_presenter.getCurrentMode() == 1) _presenter.loadData();
      });
  }

  _selectEndDate(context) async {
    final DateTime picked = await showDatePicker(
      context: context,
      initialDate: _presenter.getEndTime(), // Refer step 1
      firstDate: DateTime(2000),
      lastDate: DateTime(2025), 
    );
    if (picked != null && picked != _presenter.getEndTime())
      setState(() {
        _presenter.setEndTime(picked);
        if (_presenter.getCurrentMode() == 1) _presenter.loadData();
      });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(widget.title),
        ),
        body: SingleChildScrollView(
          child: new Center(
            child: new Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: <Widget>[
                  SizedBox(
                      height: 450,
                      child: (entryList == null || entryList.isEmpty)
                          ? Container()
                          : new charts.TimeSeriesChart(seriesList,
                              animate: animate,
                              behaviors: [new charts.PanAndZoomBehavior(),new charts.SeriesLegend(),],
                              dateTimeFactory:
                                  const charts.LocalDateTimeFactory())),
                  SizedBox(height: 10),
                  new Column(
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        new Row(
                            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                            children: <Widget>[
                              new RaisedButton(
                                child: new Text('Pick Start Date'),
                                onPressed: () => _selectStartDate(context),
                              ),
                              new RaisedButton(
                                child: new Text('Pick End Date'),
                                onPressed: () => _selectEndDate(context),
                              ),
                            ]),
                      ]),
                  SizedBox(height: 10),
                  Padding(
                      padding: EdgeInsets.all(10.0),
                      child: new TextFormField(
                        maxLines: null,
                        textInputAction: TextInputAction.done,
                        onFieldSubmitted: (value) {
                          print(value);
                          if (_presenter.setEntryCount(value) == true &&
                              _presenter.getCurrentMode() == 0)
                            _presenter.loadData();
                        },
                        decoration: InputDecoration(
                          border: OutlineInputBorder(),
                          labelText: 'Entry count',
                        ),
                      )),
                  SizedBox(height: 10),
                  new ToggleSwitch(
                    minWidth: 200.0,
                    initialLabelIndex: 0,
                    labels: ['No filter', '#0 filter','#1 filter','#2 filter'],
                    onToggle: (index) {
                      //_presenter.setFilterMode(index);
                      //_presenter.loadData();
                      print('filter switched to: $index');
                    },
                  ),
                  SizedBox(height: 10),
                  new ToggleSwitch(
                    minWidth: 200.0,
                    initialLabelIndex: 0,
                    labels: ['Query on count', 'Query on time'],
                    onToggle: (index) {
                      _presenter.setCurrentMode(index);
                      print('mode switched to: $index');
                    },
                  ),
                ]),
          ),
        ));
  }
void _showDialog(String msg) {
    // flutter defined function
    showDialog(
      context: context,
      builder: (BuildContext context) {
        // return object of type Dialog
        return AlertDialog(
          title: new Text("Exception occurred"),
          content: new Text(msg),
          actions: <Widget>[
            new FlatButton(
              child: new Text("Close"),
              onPressed: () {
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    );
  }
  @override
  void onLoadError(String msg) {
    // TODO: implement onLoadError
    _showDialog(msg);
    print('load error');
  }

  @override
  void onLoadSuccess(data) {
    print('load data success');

    setState(() {
      entryList.clear();
      entryList.addAll(data);
      print(entryList.length);
      animate = false;
      animate = true;
      //entryList = data;
    });
  }

  @override
  setPresenter(ChartPresenter presenter) {
    _presenter = presenter;
  }
}
