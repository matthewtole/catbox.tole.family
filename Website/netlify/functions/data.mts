import axios from "axios";
import type { Context } from "@netlify/functions";

const JSONBIN_KEY = process.env.JSONBIN_KEY;
const JSONBIN_ID = "5eae3b8a47a2266b1471cb78";

interface PanelData {
  lastPressed: number;
  history: number[];
}

interface Data {
  water: PanelData;
  food: PanelData;
  poop: PanelData;
}

const ID_TO_KEY = {
  0: "food",
  1: "poop",
  2: "water",
};

export default async (req: Request, context: Context) => {
  const res = await axios({
    method: "GET",
    url: `https://api.jsonbin.io/v3/b/${JSONBIN_ID}`,
    headers: {
      "X-Access-Key": JSONBIN_KEY,
    },
  });
  return new Response(res.data.record);
};
