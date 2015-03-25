'use strict';

angular.module('app').controller('sensorsController', function($scope, sensorsFactory, sensorsService, serverService) {

    $scope.data = sensorsService;

    $scope.s = {
        name: "sensy",
        value: 2
    }

    var eventSourceCallback = function(idx) {
        return function(event) {
            $scope.data.sensors[idx].value = event.data;
            console.log("sid " + $scope.data.sensors[idx].id + " received sse " + event.data);
            $scope.$apply();
        }
    }

    sensorsFactory.getAll(function() {
        for (var i = 0; i < $scope.data.sensors.length; i++) {
            var s = $scope.data.sensors[i];
            console.log("sensors: " + s.id);
            var source = new EventSource(serverService.server + "sse/sensor/" + s.id);
            source.onmessage = eventSourceCallback(i);
        }
    });

    $scope.doAction = function(sensor) {
        console.log('do action');
        var val = (parseInt(sensor.value) + 1) % 2;
        sensorsFactory.doAction({
            id: sensor.id,
            action: val
        });
    }

    $scope.toggleFavorite = function(sensor) {
        console.log('toggle favorite');
        sensorsService.toggleFavorite(sensor);
    }
});
