'use strict';

function ActivityInfo(id) {
    this.id = Number(id);

    Object.defineProperty(this, 'name', {
        configurable: false,
        get: function() { return app.activityInfoName(this.id); }
    });
}

function Activity(id) {
    this.id = Number(id);

    Object.defineProperty(this, 'intervals', {
        configurable: false,
        get: function() { return app.activityIntervals(this.id); }
    });

    Object.defineProperty(this, 'info', {
        configurable: false,
        get: function() { return new ActivityInfo(app.activityActivityInfoId(this.id)); }
    })
}

var durations = {};

app.forEachActivity(function(activityId) {
    var activity = new Activity(activityId);
    var infoId = activity.info.id;
    if (!durations.hasOwnProperty(infoId)) {
        durations[infoId] = 0.0;
    }

    var intervals = activity.intervals;
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
    var info = new ActivityInfo(infoId);

    total += duration;
    out = out + "\n" + info.name + ": " + duration + " seconds";
}
out = out + "\n" + "total: " + total + " seconds";
app.showMessageBox(out);