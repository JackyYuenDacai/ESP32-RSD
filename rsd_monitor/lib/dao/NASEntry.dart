
import 'dart:ffi';
// ignore: non_constant_identifier_names
class NASEntry{
  DateTime time_index;
  int milli_mul;
  int milli_index;
  double avg_amp;
  double max_amp;
  double min_amp;
}

class TimeSeriesAmp {
  final DateTime time;
  final Float amp;

  TimeSeriesAmp(this.time, this.amp);
}