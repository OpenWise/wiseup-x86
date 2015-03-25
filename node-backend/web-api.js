const   express       = require('express');
const   config        = require('configure');
const   ConsoleDebug  = require('console-debug');
var     console       = new ConsoleDebug(config.console);
const   RedisAdapter  = require('./redis_adapter.js')(console);
var     jsDB          = [];

var redis   = new RedisAdapter(config.sensorChannel, config.redisPort, config.redisHost, config.redisOpt);

redis.on(config.sensorChannel, function(sensorData, type) {
    for	(index = 0; index < jsDB.length; index++) {
        if (jsDB[index].id == sensorData.id) {
            jsDB[index] = sensorData;
            return;
        }
    }
    jsDB.push (sensorData);
});

var app = express();

//enable CORS
app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE,OPTIONS');
    next();
});

var apiRouter = express.Router();
var sseRouter = express.Router();

app.use('/api', apiRouter);
app.use('/sse', sseRouter);

apiRouter.route('/').get(function(req, res) {
    res.end('hello wiseup');
});

apiRouter.route('/sensors').get(function(req, res) {
    var data = [];
    for	(index = 0; index < jsDB.length; index++) {
        var dataExpand = jsDB[index];
        dataExpand.name = "new sensor";
        data.push (dataExpand);
    }
    
    console.log(data);
    res.json(data);
});

apiRouter.route('/sensors/:id/:action').get(function(req, res) {
    var data_t = {
        id: req.param('id'),
        action: req.param('action')
    };
    var data = '{"id":' + req.param('id') + ',"action":' + req.param('action') + '}';
    console.log(data);
    
    redis.Publish("SENSOR-ACTION", data, function(err, info) {
        if (err) {
            console.error("ERROR");
            res.status(500).send(err.message);
        } else {
            console.error("GOOD");
            res.json(data_t);
        }
    }); // TODO error handling
    
    res.json(data_t);
});

var msgid = 0;

sseRouter.route('/sensor/:id').get(function(req, res) {
    var sid = req.params.id;
    console.log("received new listener for sensor id " + sid);
    req.socket.setTimeout(Infinity);
    res.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });
    res.write('\n');

    var writeSse = function(d) {
        res.write("id: " + msgid++ +"\n");
        res.write("data: " + d);
        res.write("\n\n");
    }

    redis.GetSensorValue(sid, function(err, data) {
        if (err) {
            console.error(err.message);
        } else {
            if (data) {
                console.log("initial sse value from redis " + sid + " " + data);
                var sensor = JSON.parse(data);
                writeSse(sensor.value);
            } else {
                console.log("sensor value not found in redis: " + sid);
            }
        }
    });
    var onSensorValueUpdate = function(data) {
        console.log("received sensor value event for id " + sid + ": " + data);
        writeSse(data);
    }
    var subid = redis.on(sid, onSensorValueUpdate);
    req.on("close", function() {
        redis.removeListener(sid, onSensorValueUpdate);
    });
});

var server = app.listen(config.serverPort, function() {
    console.info('Listening on port ' + server.address().port);
});
