self.addEventListener('message', function (e) {
    console.log("worker started");
    let list = JSON.parse(e.data[0]);
    let result = [];

    for (let i = 0; i < list.length; i += e.data[1])
        result.push(
            list.slice(i, i + e.data[1])
        );
    self.postMessage(result);
}, false);