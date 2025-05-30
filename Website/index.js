const { DateTime } = luxon;

const refresh = document.getElementById("refresh");

const panels = {
  poop: {
    element: document.querySelector(".panel[data-id='poop']"),
    delay: 8 * 60 * 60,
  },
  water: {
    element: document.querySelector(".panel[data-id='water']"),
    delay: 7 * 24 * 60 * 60,
  },
  food: {
    element: document.querySelector(".panel[data-id='food']"),
    delay: 14 * 24 * 60 * 60,
  },
};

function updatePanel(id, data) {
  const element = panels[id].element;
  element.querySelector(".time").innerText = DateTime.now()
    .minus({ seconds: data })
    .toRelative();
  if (data >= panels[id].delay) {
    element.setAttribute("data-mode", "on");
  } else {
    element.setAttribute("data-mode", "off");
  }
}

async function fetchData() {
  if (refresh.classList.contains("active")) {
    return;
  }
  refresh.classList.add("active");
  const res = await axios.get(
    "https://matthewtole--5aad334ee66c11ef8859e6cdfca9ef9f.web.val.run/status"
  );
  updatePanel("poop", res.data.poop);
  updatePanel("water", res.data.water);
  updatePanel("food", res.data.food);
  refresh.classList.remove("active");
}

function boot() {
  setInterval(fetchData, 5000);
  fetchData().catch((err) => console.error(err));
  refresh.addEventListener("click", fetchData);
}

boot();
