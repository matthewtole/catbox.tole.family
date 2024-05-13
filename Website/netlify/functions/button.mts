
import axios from "axios";
import type { Context } from "@netlify/functions"

const JSONBIN_KEY = process.env.JSONBIN_KEY;
const JSONBIN_ID = "5eae3b8a47a2266b1471cb78";

export default async (req: Request, context: Context) => {
  // get id query param from the request url using UrlSearchParams
  const url = new URL(req.url);
  const id = url.searchParams.get("id");
  const res = await axios({
    method: "GET",
    url: `https://api.jsonbin.io/v3/b/${JSONBIN_ID}`,
    headers: {
      "X-Access-Key": JSONBIN_KEY,
    },
  });
  return new Response(JSON.stringify(res));
};
