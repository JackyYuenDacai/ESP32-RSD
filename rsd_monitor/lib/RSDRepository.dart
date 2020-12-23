import 'dart:async';
import 'package:mysql1/mysql1.dart';
import 'dao/NASEntry.dart';
import 'package:intl/intl.dart';

//List<charts.Series<TimeSeriesSales, DateTime>>
class RSDRepository {
  var _settings = new ConnectionSettings(
      host: '45.63.35.198',
      port: 3306,
      user: 'dlm_user',
      password: '-Sm4Dgt:q#?9t(qv',
      db: 'dlm_db');
	  
  init() async {

  }

  Future<List<NASEntry>> loadDataByCount(int entryCount) async {
    List<NASEntry> ret = new List<NASEntry>();
    final _conn = await MySqlConnection.connect(_settings);
    var results = await _conn.query(
        'select time_index,milli_mul,milli_index,avg_amp,max_amp,min_amp from NAS order by time_index desc limit 0,' +
            entryCount.toString() +
            ';');
    for (var row in results) {
      var entry = new NASEntry();
      entry.time_index = row[0];
      entry.milli_mul = row[1];
      entry.milli_index = row[2];
      entry.avg_amp = row[3];
      entry.max_amp = row[4];
      entry.min_amp = row[5];
      ret.add(entry);
    }
    await _conn.close();
    return ret;
  }

  Future<List<NASEntry>> loadDataByDate(
      DateTime startDate, DateTime endDate) async {
    final DateFormat formatter = DateFormat('yyyy-MM-dd HH:mm:ss');
    final String start = formatter.format(startDate);
    final String end = formatter.format(endDate);
    List<NASEntry> ret = new List<NASEntry>();
    final _conn = await MySqlConnection.connect(_settings);
    var results = await _conn.query(
        'select time_index,milli_mul,milli_index,avg_amp,max_amp,min_amp from NAS where time_index <= \"' +
            end +
            '\" and time_index >= \"' +
            start +
            '\" order by time_index desc ;');
    for (var row in results) {
      var entry = new NASEntry();
      entry.time_index = row[0];
      entry.milli_mul = row[1];
      entry.milli_index = row[2];
      entry.avg_amp = row[3];
      entry.max_amp = row[4];
      entry.min_amp = row[5];
      ret.add(entry);
    }
    await _conn.close();
    return ret;
  }
}
