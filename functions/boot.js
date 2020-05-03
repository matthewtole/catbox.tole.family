const axios = require('axios');
var Particle = require('particle-api-js');
var particle = new Particle();

exports.handler = function(event, context, callback) {
  axios
    .get(process.env.BIN_URL, {
      headers: {
        'secret-key': process.env.BIN_TOKEN,
      },
    })
    .then(({ data }) => {
      return particle.callFunction({
        deviceId: process.env.PARTICLE_DEVICE,
        name: 'setFoodTime',
        argument: `${data.food.lastPressed}`,
        auth: process.env.PARTICLE_TOKEN,
      });
    })
    .catch(ex => console.error(ex))
    .finally(() => {
      callback();
    });
};
