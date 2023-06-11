function get(url, headers) {
    return new Promise(function (resolve, reject) {
        let xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            try{
                if (xmlhttp.readyState !== XMLHttpRequest.DONE) {
                    return;
                }
                let headers = xmlhttp.getAllResponseHeaders()
                if(xmlhttp.status === 200) {
                    Qt.callLater(resolve, {"headers": headers, "body": xmlhttp.responseText});
                }else{
                    reject({"status": xmlhttp.status, "statusText": xmlhttp.statusText, "headers": headers});
                }
            }catch(err) {
                reject(err);
            }
        };
        if (headers) {
            for (let header in headers) {
                xmlhttp.setRequestHeader(header, headers[header]);
            }
        }
        xmlhttp.open("GET", url);
        xmlhttp.send();
    });
}

function post(url, headers, body) {
    return new Promise(function (resolve, reject) {
        let xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            try{
                if (xmlhttp.readyState !== XMLHttpRequest.DONE) {
                    return;
                }
                let headers = xmlhttp.getAllResponseHeaders()
                if(xmlhttp.status === 200) {
                    Qt.callLater(resolve, {"headers": headers, "body": xmlhttp.responseText});
                }else{
                    reject({"status": xmlhttp.status, "statusText": xmlhttp.statusText, "headers": headers});
                }
            }catch(err) {
                reject(err);
            }
        };

        xmlhttp.open("POST", url);
        if (headers) {
            for (let header in headers) {
                xmlhttp.setRequestHeader(header, headers[header]);
            }
        }
        xmlhttp.send(body);
    });
}
