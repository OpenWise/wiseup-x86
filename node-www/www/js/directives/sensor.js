'use strict';

var TEMPERATURE_TYPE = 1;
var LUMINANCE_TYPE = 2;
var MOVEMENT_TYPE = 3;
var SWITCH_TYPE = 4;

angular.module('app').directive('sensor', function($http) {
    return {
        restrict: 'E',
        replace: true,
        scope: {
            data: '=',
            onAction: '&',
            onToggleFavorite: '&'
        },
        templateUrl: 'views/templates/sensor.html'
    }
});

angular.module('app').directive('toggleButton', function($http) {
    return {
        require: 'ngModel',
        scope: {
            sensorId: '@sensorId',
            sensorVal: '@sensorVal',
            activeText: '@activeText',
            inactiveText: '@inactiveText',
            sensorState: '=ngModel'
        },
        replace: true,
        transclude: true,
        template: '<div>' +
            '<button class="pure-button" ng-class="{\'pure-button-primary\': state.value}" ng-click="state.toggle()">{{activeText}}</button>' +
            '<button class="pure-button" ng-class="{\'pure-button-primary\': !state.value}" ng-click="state.toggle()">{{inactiveText}}</button>' +
            '</div>',
        link: function postLink(scope) {
            scope.sensorState = "BLACK";
            scope.state = {
                value: false,
                color: "BLACK",
                toggle: function() {
                    this.value = !this.value;
                    this.color = (this.value == true) ? "RED" : "BLACK";
                    scope.sensorState = this.color;
                    SendCommand($http, scope.sensorId, (this.value == true) ? 1 : 0);
                }
            };

            scope.sensorState = (scope.sensorVal == 1) ? "RED" : "BLACK";
            scope.state.value = (scope.sensorVal == 1) ? true : false;
        }
    }

    function SendCommand($http, sensorId, action) {
        $http.get('http://10.0.0.15/api/sensors/' + sensorId + '/' + action).
        success(function(data, status, headers, config) {}).
        error(function(data, status, headers, config) {});
    }
});
