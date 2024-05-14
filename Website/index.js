async function fetchData() {
  const res = await axios.get("/.netlify/functions/data");
  console.log(res.data);
}

fetchData().catch((err) => console.error(err));
