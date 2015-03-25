'use strict';

angular.module('app').factory('sensorsFactory', function($resource, sensorsService, serverService) {
    return $resource(serverService.server + 'api/sensors/:id/:action', null, {
        getAll: {
            method: "GET",
            isArray: true,
            interceptor: {
                response: function(response) {
                    sensorsService.sensors = response.data;
                    //todo: add favorite from API this is just mock
                    sensorsService.sensors.forEach(function(s) {
                        s.favorite = false;
                    });
                    return response.data;
                }
            }
        },
        doAction: {
            method: "GET",
            //url: serverService.server + "api/sensors/:id/:action",
            interceptor: {
                response: function(response) {
                    sensorsService.setValue(response.data);
                    return response.data;
                }
            }
        }
    });
});
