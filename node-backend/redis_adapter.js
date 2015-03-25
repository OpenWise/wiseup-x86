const redis  = require('redis');
const events = require('events');
const moment = require('moment');
var console;
//{id:1108152157441,hub:4328719365,addr:X,type:1,value:20}
function RedisAdapter(sensorChannel, port, host, options) {
    this.sub = redis.createClient(port, host, options);
    this.pub = redis.createClient(port, host, options);
    this.client = redis.createClient(port, host, options);
    events.EventEmitter.call(this);
    
    // Clear REDIS keys
    this.client.flushdb ( function (err, didSucceed) {
        console.log("REDIS FLUSHDB - " + didSucceed);
    });

    this.sub.subscribe(sensorChannel);
    var self = this;
    this.sub.on("message", function(channel, message) {
        switch (channel) {
            case sensorChannel:
                /*
                 * getting sensor info from c++ layer
                 * 
                 * { "id": "sensor_addr", 
                 *   "hub": "hub_addr", 
                 *   "addr": "sensor_local_id", 
                 *   "type" :"val", 
                 *   "value": "val" }
                 */
                var sensor = JSON.parse(message);
                /* add time-stamp to the structure */
                sensor.ts = moment().unix();
                /* print the basic data to console */
                console.log( "{ CH:" + channel + 
                             ", ID:" + sensor.id + 
                             ", VALUE: " + sensor.value + " }");
                self.client.get ("" + sensor.id, function (err, data) {
                    if (err) {
                        console.error(err.message);
                    } else {
                        if (data) {
                            // Check if sensor data was changed
                            var redisSensor = JSON.parse(data);
                            if (redisSensor.value == sensor.value) { // Do nothing
                            } else { // Send Events
                                self.emit(sensor.id, sensor.value);
                                self.emit(sensorChannel, sensor, "OLD"); // this.on
                            }
                        } else {
                            self.emit(sensorChannel, sensor, "NEW");
                        }
                        
                        self.client.set (sensor.id, message, function(err, reply) {
                            if (err) {
                                console.error(err.message);
                            }
                        });
                    }
                });
                break;
        }
    });
    console.info("REDIS ADAPTER: subscribed for channel " + sensorChannel);
}

RedisAdapter.prototype.GetSensorValue = function(sid, callback) {
    this.client.get("" + sid, callback);
}

RedisAdapter.prototype.GetSensors = function(callback) {
    this.client.keys('*', callback);
}

RedisAdapter.prototype.Publish = function(channel, data, callback) {
    this.pub.publish(channel, data);
}

RedisAdapter.prototype.__proto__ = events.EventEmitter.prototype;

function RedisFactory(c) {
    console = c;
    return RedisAdapter;
}

module.exports = RedisFactory;
