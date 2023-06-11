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

    exports.Q = {
        shot: shot,
        isItem: _instanceOfItem,
        isPopup: _instanceOfPopup,
    };

})(this);
