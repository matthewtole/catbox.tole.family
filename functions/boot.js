const CacheAsset = require('@11ty/eleventy-cache-assets');
var Particle = require('particle-api-js');
var particle = new Particle();

exports.handler = function(event, context, callback) {
  CacheAsset(process.env.BIN_URL, {
    duration: '1h',
    type: 'json',
    fetchOptions: {
      headers: {
        'secret-key': process.env.BIN_TOKEN,
      },
    },
  })
    .then(data => {
      return particle.callFunction({
        deviceId: process.env.PARTICLE_DEVICE,
        name: 'setFoodTime',
        argument: `${data.food.lastPressed}`,
        auth: process.env.PARTICLE_TOKEN,
      });
    })
    .finally(() => {
      callback();
    });
};
