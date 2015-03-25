'use strict';

angular.module('app').service('sensorsService', function() {
    this.sensors = [];

    this.findAndDo = function(sensor, callback) {
        for (var i = 0; i < this.sensors.length; i++) {
            if (this.sensors[i].id == sensor.id) {
                callback(this.sensors[i]);
                return;
            }
        }
    }

    this.toggleFavorite = function(sensor) {
        this.findAndDo(sensor, function(snsr) {
            snsr.favorite = !snsr.favorite;
        });
    }

    this.setValue = function(sensor) {
        this.findAndDo(sensor, function(snsr) {
            snsr.value = sensor.value;
        });
    }

});
