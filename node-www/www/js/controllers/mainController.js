'use strict';

angular.module('app').controller('mainController', function($scope, $http, cordovaService) {
    $scope.ready = false;
    $scope.isCordovaApp = undefined;
    $scope.sensor1 = "no data yet";
    
    cordovaService.ready.then(
        function resolved(resp) {
            $scope.isCordovaApp = true;
            $scope.ready = true;
        },
        function rejected(resp) {
            $scope.isCordovaApp = false;
            $scope.ready = true;
        }
    );
    
    

    // var source = new EventSource("http://10.100.102.9:5678/sse/sensor/1");

    // source.onmessage = function(event) {
    //     $scope.sensor1 = event.data;
    //     console.log("received sse " + event.data);
    //     $scope.$apply();
    // };

    // source.addEventListener('message', function(e) {
    //     $scope.sensor1 = e.data;
    //     console.log("received sse " + e.data);
    //     $scope.$apply();
    // }, false);

});
