// async.js


function localCommand(local, cmd, fnAbort) {
    return new Promise(function (resolve, reject) {
        let fnCommandFinished = function(){
            local.commandFinished.disconnect(fnCommandFinished);
            if(typeof(fnAbort) !== "function") {
                Qt.callLater(local.lastExitCode !== 0 ? reject : resolve);
                return
            }
            Qt.callLater(fnAbort(local.lastExitCode) === true ? reject : resolve);
        }
        local.commandFinished.connect(fnCommandFinished);
        local.command = cmd;
        local.start();
    });
}

function remoteCommand(ssh, cmd, fnAbort) {
    //console.log("remoteCommand", ssh, cmd)
    return new Promise(function (resolve, reject) {
        let hasError = false;
        let fnSubCommandAbort = function(host, code){
            if(typeof(fnAbort) !== "function") {
                if(code !== 0) {
                    hasError = true;
                    ssh.abort();
                }
                return
            }
            if(fnAbort(host, code) === true) {
                hasError = true;
                ssh.abort();
            }
        }
        ssh.subcommandFinished.connect(fnSubCommandAbort);
        let fnCommandFinished = function(host){
            ssh.subcommandFinished.disconnect(fnSubCommandAbort);
            ssh.commandFinished.disconnect(fnCommandFinished);
            Qt.callLater(hasError ? reject : resolve);
        }
        ssh.commandFinished.connect(fnCommandFinished);
        ssh.command = cmd;
        ssh.start();
    });
}

function bigScriptCommand(ssh, script, fnAbort) {
    return new Promise(function (resolve, reject) {
        let hasError = false;
        let fnSubCommandAbort = function(host, code){
            if(typeof(fnAbort) !== "function") {
                if(code !== 0) {
                    hasError = true;
                    ssh.abort();
                }
                return
            }
            if(fnAbort(host, code) === true) {
                hasError = true;
                ssh.abort();
            }
        }
        ssh.subcommandFinished.connect(fnSubCommandAbort);
        let fnCommandFinished = function(host){
            ssh.subcommandFinished.disconnect(fnSubCommandAbort);
            ssh.commandFinished.disconnect(fnCommandFinished);
            Qt.callLater(hasError ? reject : resolve);
        }
        ssh.commandFinished.connect(fnCommandFinished);
        ssh.script = script;
        ssh.start();
    });
}

function fileTransferCommand(sftp, fileLocal, fileRemote, fileAppend, fileDownload, fnAbort) {
    return new Promise(function (resolve, reject) {
        let hasError = false;
        let fnSubCommandAbort = function(host, code){
            if(typeof(fnAbort) !== "function") {
                if(code !== 0) {
                    hasError = true;
                    sftp.abort();
                }
                return
            }
            if(fnAbort(host, code) === true) {
                hasError = true;
                sftp.abort();
            }
        }
        sftp.subcommandFinished.connect(fnSubCommandAbort);
        let fnCommandFinished = function(host){
            sftp.subcommandFinished.disconnect(fnSubCommandAbort);
            sftp.commandFinished.disconnect(fnCommandFinished);            
            Qt.callLater(hasError ? reject : resolve);
        }
        sftp.commandFinished.connect(fnCommandFinished);
        sftp.fileLocal = fileLocal;
        sftp.fileRemote = fileRemote;
        sftp.fileAppend = fileAppend;
        sftp.fileDownload = fileDownload;
        sftp.start();
    });
}

function fileListCommand(sftp, path, fnAbort) {
    return new Promise(function (resolve, reject) {
        let hasError = false;
        let fnSubCommandAbort = function(host, code){
            if(typeof(fnAbort) !== "function") {
                if(code !== 0) {
                    hasError = true;
                    sftp.abort();
                }
                return
            }
            if(fnAbort(host, code) === true) {
                hasError = true;
                sftp.abort();
            }
        }
        sftp.subcommandFinished.connect(fnSubCommandAbort);
        let fnCommandFinished = function(host){
            sftp.subcommandFinished.disconnect(fnSubCommandAbort);
            sftp.commandFinished.disconnect(fnCommandFinished);
            Qt.callLater(hasError ? reject : resolve);
        }
        sftp.commandFinished.connect(fnCommandFinished);
        sftp.path = path;
        sftp.start();
    });
}

function fileContentCommand(sftp, path, contentOrReadOnly, fnAbort) {
    return new Promise(function (resolve, reject) {
        let hasError = false;
        let fnSubCommandAbort = function(host, code){
            if(typeof(fnAbort) !== "function") {
                if(code !== 0) {
                    hasError = true;
                    sftp.abort();
                }
                return
            }
            if(fnAbort(host, code) === true) {
                hasError = true;
                sftp.abort();
            }
        }
        sftp.subcommandFinished.connect(fnSubCommandAbort);
        let fnCommandFinished = function(host){
            sftp.subcommandFinished.disconnect(fnSubCommandAbort);
            sftp.commandFinished.disconnect(fnCommandFinished);
            Qt.callLater(hasError ? reject : resolve);
        }
        sftp.commandFinished.connect(fnCommandFinished);
        sftp.filePath = path;
        if(contentOrReadOnly === true) {
            sftp.content = "";
            sftp.fileRead = true;
        }else{
            sftp.content = contentOrReadOnly
            sftp.fileRead = false;
        }
        sftp.start();
    });
}



function throwCommand() {
    return new Promise(function (resolve, reject) {
        Qt.callLater(reject);
    });
}

function asyncGenerator(fn) {
    return function() {
        var thiz = this,
        args = arguments
        return new Promise(function(resolve, reject) {
            var gen = fn.apply(thiz, args)
            function _next(value) {
                step("next", value)
            }
            function _throw(err) {
                step("throw", err)
            }
            function step(key, arg) {
                try {
                    var info = gen[key](arg)
                    var value = info.value
                } catch (error) {
                    reject(error)
                    return
                }
                if(info.done) {
                    resolve(value)
                } else {
                    Promise.resolve(value).then(_next, _throw)
                }
            }
            _next(undefined)
        })
    }
}
