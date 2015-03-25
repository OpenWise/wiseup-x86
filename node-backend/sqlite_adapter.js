var sqlite3 = require('sqlite3').verbose();
var console;

//TODO: protect against SQL injection (escape user input)

function SqliteAdapter(dbFile) {
    var self = this;
    this.db = new sqlite3.Database(dbFile);
    
    var sql = this.db;
    sql.serialize(function() {
        sql.run("CREATE TABLE IF NOT EXISTS `sensor_info` (" +
            "`sensor_id`                UNSIGNEDBIGINT NOT NULL PRIMARY KEY," +
            "`sensor_hub_address`       UNSIGNEDBIGINT NOT NULL," +
            "`sensor_address`           SMALLINT NOT NULL," +
            "`sensor_name`              VARCHAR(128) NOT NULL," +
            "`sensor_family_type`       SMALLINT NOT NULL," +
            "`registration_datetime`    INTEGER NOT NULL," +
            "`update_interval`          SMALLINT NOT NULL," +
            "`available`                TINYINT NOT NULL);"); //remove available it is redundant

        sql.run("CREATE TABLE IF NOT EXISTS `sensor_data_history` (" +
            "`record_id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
            "`sensor_id`                UNSIGNEDBIGINT NOT NULL," +
            "`time_stamp`               INTEGER NOT NULL," +
            "`value`                    INTEGER NOT NULL);");

        sql.run("CREATE TABLE IF NOT EXISTS `sensor_data` (" +
            "`sensor_id`                UNSIGNEDBIGINT NOT NULL PRIMARY KEY," +
            "`time_stamp`               UNSIGNEDBIGINT NOT NULL," +
            "`value`                    INTEGER NOT NULL);");
    });
}

SqliteAdapter.prototype.GetAllSensors = function(callback) {
    var sql = this.db;
    sql.serialize(function() {
        var query = "SELECT SI.sensor_id AS id, SI.sensor_hub_address AS hub, SI.sensor_name AS name, SI.sensor_family_type AS type, SD.time_stamp AS ts, SD.value, SI.available " +
            "FROM  `sensor_info` SI " +
            "JOIN `sensor_data` SD ON SI.sensor_id = SD.sensor_id " +
            "WHERE SI.available = '1';";

        sql.all(query, function(err, rows) {
            callback(null, rows);
        });
    });
}

SqliteAdapter.prototype.CreateNewSensor = function(sensor) {

    var sql = this.db;
    sql.serialize(function() {
        var query = "INSERT INTO `sensor_info` (`sensor_id`, " +
            "`sensor_hub_address`, " +
            "`sensor_address`, " +
            "`sensor_name`, " +
            "`sensor_family_type`, " +
            "`registration_datetime`, " +
            "`update_interval`, " +
            "`available`) " +
            "VALUES (" + sensor.id + "," +
            sensor.hub + "," +
            sensor.addr + "," +
            "'New sensor'," +
            sensor.type + "," +
            "strftime('%s','now')," +
            "30," +
            "1);";
        sql.run(query);

        query = "INSERT INTO `sensor_data` (`sensor_id`, " +
            "`time_stamp`, " +
            "`value`) " +
            "VALUES (" + sensor.id + "," +
            sensor.ts + "," +
            sensor.value + ");";
        console.log(query);
        sql.run(query);
    });
}

SqliteAdapter.prototype.UpdateSensorValue = function(sensor) {
    var sql = this.db;
    sql.serialize(function() {
        sql.run("UPDATE `sensor_data` SET `value`=" + sensor.value + ", `time_stamp`=" + sensor.ts + " WHERE `sensor_id`=" + sensor.id);
    });
}

SqliteAdapter.prototype.ArchiveSensorValue = function(sensor) {
    var sql = this.db;
    sql.serialize(function() {
        sql.run("INSERT INTO `sensor_data_history` (`record_id`, `sensor_id`, `time_stamp`, `value`) " +
            "VALUES (NULL," + sensor.id + "," + sensor.ts + "," + sensor.value + ");");
    });
}

function SqliteFactory(c) {
    console = c;
    return SqliteAdapter;
}

module.exports = SqliteFactory;
