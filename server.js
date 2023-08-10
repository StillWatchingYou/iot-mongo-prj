const express = require('express');
const bodyParser = require('body-parser');
const { MongoClient } = require('mongodb');

const app = express();
const port = 3000;

app.use(express.static('public'));
app.use(bodyParser.json());

let dataStore = [];
let value1 =  [] ;

async function pushdata(newData) {
  const uri = "mongodb+srv://doducmanhctb:Manh2710@cluster0.8d4w1lx.mongodb.net/"; // Thay đổi thông tin MongoDB của bạn

  const client = new MongoClient(uri);

  try {
    await client.connect();

    const db = client.db('node_test');
    const collection = db.collection('test');

    await collection.insertOne(newData);
    console.log(newData);

  } catch (e) {
    console.error(e);
  } finally {
    await client.close();
  }
}

app.post('/api/data', (req, res) => {
  const { temperature, humidity, timestamp } = req.body; 

  console.log(`Received data - Temperature: ${temperature}, Humidity: ${humidity}, Timestamp: ${timestamp}`);

  dataStore.push({ temperature, humidity, timestamp });

  res.status(200).send('Data saved successfully');

  const newData = {
    Temp: temperature,
    Humi: humidity,
    Timestamp: timestamp,
  };
  pushdata(newData);
});

app.post('/api/control', (req, res) => {
  const { value } = req.body;
  value1.shift();
  value1.push({ value });
  console.log('Received control value:', value);
  res.status(200).send('Control signal received');
});

app.get('/api/control', (req, res) => {
  res.json(value1);
});


app.get('/api/data', (req, res) => {
  res.json(dataStore);
});

app.listen(port, () => {
  console.log(`Server listening on port ${port}`);
});
