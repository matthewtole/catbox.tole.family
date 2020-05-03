const CacheAsset = require('@11ty/eleventy-cache-assets');

module.exports = async function() {
  return await CacheAsset(process.env.BIN_URL, {
    duration: '1h',
    type: 'json',
    fetchOptions: {
      headers: {
        'secret-key': process.env.BIN_TOKEN,
      },
    },
  });
};
