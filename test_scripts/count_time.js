'use strict';


function formatMs(ms) {
    var secs = ms / 1000;
    var mins = secs / 60;
    var hours = mins / 60;
    var days = hours / 24;

    return [Math.floor(days), Math.floor(hours), Math.floor(mins), (secs)].join(':');
}

var durations = {};

app.forEachActivity(function(activityId) {
	var infoId = app.activityActivityInfoId(activityId);
	if (!durations.hasOwnProperty(infoId)) {
		durations[infoId] = 0.0;
	}

	var intervals = app.activityIntervals(activityId);
	app.debugLog(intervals.length + ' length is')
	for (var i = 0; i < intervals.length; i += 2) {
		var startTime = intervals[i];
		var endTime = intervals[i+1];
		var dur = (endTime - startTime) / 1000.0;
		app.debugLog('duration of' + activityId + 'is ' + dur)
		app.debugLog('ss ' + startTime + ' es ' + endTime);
		durations[infoId] = durations[infoId] + dur;
	}
});

var out = "";
var total = 0;
for (var infoId in durations) {
	var duration = durations[infoId];
	var infoName = app.activityInfoName(Number(infoId));

	total += duration;
	out = out + "\n" + infoName + ":" + formatMs(duration);
}
out = out + "\n" + "total: " + formatMs(total);
app.showMessageBox(out);