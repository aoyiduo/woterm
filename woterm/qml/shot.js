// shot.js

(function(exports) {
    function _instanceOfSignal(object) {
        return typeof(object) === "function" &&
                typeof(object.hasOwnProperty) === "function" &&
                typeof(object.connect) === "function" &&
                typeof(object.disconnect) === "function";
    }

    function _hasVisibleProperty(object) {
        return typeof(object) === "object" && typeof(object.visibleChanged) === "function";
    }

    function _instanceOfItem(object) {
        return typeof(object) === "object" && typeof(object.mapFromItem) === "function";
    }

    function _instanceOfPopup(object) {
        return typeof(object) === "object" && typeof(object.modalChanged) === "function";
    }

    function Promise() {
        this._callbacks = [];
    }

    Promise.prototype.then = function(func, context) {
        var p;
        if (this._isdone) {
            p = func.apply(context, this.result);
        } else {
            p = new Promise();
            this._callbacks.push(function () {
                var res = func.apply(context, arguments);
                if (res && typeof res.then === 'function'){
                    res.then(p.done, p);
                }
            });
        }
        return p;
    };

    Promise.prototype.done = function() {
        this.result = arguments;
        this._isdone = true;
        for (var i = 0; i < this._callbacks.length; i++) {
            this._callbacks[i].apply(null, arguments);
        }
        this._callbacks = [];
    };

    function shot(funcSignal, funcb, item, context) {
        if(!_instanceOfSignal(funcSignal)) {
            throw new Error("the first parameter should be a signal function.");
        }
        if(typeof(funcb) !== "function") {
            throw new Error("the second parameter should be a function.");
        }

        let func = function() {
            funcSignal.disconnect(func)
            funcb.apply(context, arguments)
        }
        funcSignal.connect(func)

        if(_hasVisibleProperty(item)) {
            let fnclr = function() {
                if(!item.visible) {
                    funcSignal.disconnect(func)
                    item.visibleChanged.disconnect(fnclr)
                }
            }
            item.visibleChanged.connect(fnclr)
        }else if(_instanceOfSignal(item)) {
            let fnclr = function() {
                funcSignal.disconnect(func)
                item.disconnect(fnclr)
            }
            item.connect(fnclr)
        }
    }

    function promise() {
        var p = new Promise()
        return p;
    }

    exports.Q = {
        Promise: Promise,
        promise: promise,
        shot: shot,
        isItem: _instanceOfItem,
        isPopup: _instanceOfPopup,
    };

})(this);
