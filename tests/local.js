import express from 'express';
import path from 'path';

const app = express();

var dir = path.join(import.meta.dirname, "..");

app.use(
  "/assets",
  express.static(path.join(dir, "web/assets"), {
    setHeaders: (res, path) => 
      res.setHeader("Cross-Origin-Embedder-Policy", "require-corp")
    ,
  })
);

app.get("/", (req, res) => {
  res.sendFile(path.join(dir, "web/index.html"));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on port http://localhost:${PORT}`);
});
