function Activity(id) {
    if (!Number.isInteger(id))
        throw TypeError('id should be integer');
    this._id = id;
}

Object.defineProperty(Activity.prototype, 'id', {
    get: function() { return this._id; },
    set: function(value) { /* typecheck */ this._id = value; },
    configurable: false
});

Object.defineProperty(Activity.prototype, 'displayString', {
    get: function() { return this._id.toString() + ' blah'; },
    configurable: false
});
