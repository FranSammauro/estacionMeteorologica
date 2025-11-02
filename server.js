const express = require("express")
const http = require("http")
const socketIo = require("socket.io")
const path = require("path")
const sqlite3 = require("sqlite3").verbose()
const cors = require("cors")

const app = express()
const server = http.createServer(app)
const io = socketIo(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"],
  },
})

// Middlewares
app.use(cors())
app.use(express.json())
app.use(express.static(path.join(__dirname, "public")))

// Base de datos SQLite
const db = new sqlite3.Database("./data/weather.db", (err) => {
  if (err) console.error("Error abriendo DB:", err)
  else console.log("Base de datos conectada")
})

// Crear tabla si no existe
db.run(`
  CREATE TABLE IF NOT EXISTS readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL,
    humidity REAL,
    pressure REAL,
    light INTEGER,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
  )
`)

// Variables para almacenar último dato
let lastReading = {
  temperature: 0,
  humidity: 0,
  pressure: 0,
  light: 0,
  timestamp: new Date(),
}

// Rutas API
// GET /api/add - Recibe datos del Arduino
app.get("/api/add", (req, res) => {
  const { temp, hum, pres, light } = req.query

  if (temp === undefined || hum === undefined || pres === undefined) {
    return res.status(400).json({ error: "Parámetros faltantes" })
  }

  const temperature = Number.parseFloat(temp)
  const humidity = Number.parseFloat(hum)
  const pressure = Number.parseFloat(pres)
  const lightValue = Number.parseInt(light) || 0
  const timestamp = new Date()

  // Guardar en BD
  db.run(
    "INSERT INTO readings (temperature, humidity, pressure, light) VALUES (?, ?, ?, ?)",
    [temperature, humidity, pressure, lightValue],
    (err) => {
      if (err) console.error("Error insertando:", err)
    },
  )

  // Actualizar último dato
  lastReading = { temperature, humidity, pressure, light: lightValue, timestamp }

  // Emitir por Socket.io a todos los clientes conectados
  io.emit("newReading", lastReading)

  res.json({ success: true, message: "Datos recibidos", data: lastReading })
})

// GET /api/current - Obtiene el último dato
app.get("/api/current", (req, res) => {
  res.json(lastReading)
})

// GET /api/history - Obtiene historial (últimos 100 registros)
app.get("/api/history", (req, res) => {
  db.all("SELECT * FROM readings ORDER BY timestamp DESC LIMIT 100", (err, rows) => {
    if (err) {
      return res.status(500).json({ error: err.message })
    }
    res.json(rows.reverse())
  })
})

// GET /api/stats - Estadísticas
app.get("/api/stats", (req, res) => {
  db.get(
    `SELECT 
      MAX(temperature) as maxTemp,
      MIN(temperature) as minTemp,
      AVG(temperature) as avgTemp,
      MAX(humidity) as maxHum,
      MIN(humidity) as minHum,
      MAX(pressure) as maxPres,
      MIN(pressure) as minPres
     FROM readings`,
    (err, row) => {
      if (err) {
        return res.status(500).json({ error: err.message })
      }
      res.json(row || {})
    },
  )
})

// Socket.io - Conexiones en tiempo real
io.on("connection", (socket) => {
  console.log("Cliente conectado:", socket.id)

  // Enviar último dato al conectar
  socket.emit("currentData", lastReading)

  socket.on("disconnect", () => {
    console.log("Cliente desconectado:", socket.id)
  })
})

// Servir el archivo HTML
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"))
})

// Iniciar servidor
const PORT = process.env.PORT || 3000
server.listen(PORT, () => {
  console.log(`Servidor ejecutándose en http://localhost:${PORT}`)
  console.log(`Accede en la red local: http://<IP_LOCAL>:${PORT}`)
})
