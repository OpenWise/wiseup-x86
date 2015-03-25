'use strict';

var app = angular.module('app', ['ngRoute', 'ngResource']);

app.config(['$routeProvider',
    function($routeProvider) {
        $routeProvider
            .when('/', {
                templateUrl: "/views/sensors.html",
                controller: 'sensorsController'
            }).when('/sensors', {
                templateUrl: "/views/sensors.html",
                controller: 'sensorsController'
            }).when('/dashboard', {
                templateUrl: "/views/dashboard.html"
            }).when('/widgets', {
                templateUrl: "/views/widgets.html"
            })
            .otherwise({
                redirectTo: '/'
            });
    }
]);
