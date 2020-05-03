const axios = require('axios');
var Particle = require('particle-api-js');
var particle = new Particle();

exports.handler = async function(event, context, callback) {
  const { data } = await axios.get(process.env.BIN_URL, {
    headers: {
      'secret-key': process.env.BIN_TOKEN,
    },
  });
  console.log('Got the data from JSONBin');
  await particle.callFunction({
    deviceId: process.env.PARTICLE_DEVICE,
    name: 'setFoodTime',
    argument: `${data.food.lastPressed}`,
    auth: process.env.PARTICLE_TOKEN,
  });
  console.log('Called setFoodTime');
  await particle.callFunction({
    deviceId: process.env.PARTICLE_DEVICE,
    name: 'setPoopTime',
    argument: `${data.poop.lastPressed}`,
    auth: process.env.PARTICLE_TOKEN,
  });
  console.log('Called setPoopTime');
  await particle.callFunction({
    deviceId: process.env.PARTICLE_DEVICE,
    name: 'setWaterTime',
    argument: `${data.water.lastPressed}`,
    auth: process.env.PARTICLE_TOKEN,
  });
  console.log('Called setWaterTime');
  callback();
};

if (require.main === module) {
  require('dotenv').config();
  exports.handler(null, null, console.log);
}
