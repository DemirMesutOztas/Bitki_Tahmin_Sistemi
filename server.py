from flask import Flask, request, jsonify
import joblib
import json
import numpy as np
app = Flask(__name__)
model = joblib.load("model.pkl")
scaler = joblib.load("scaler.pkl")

with open("siniflar.json", "r") as f:
    siniflar = json.load(f)

@app.route("/predict", methods=["POST"])
def predict():
    veri = request.get_json()

    özellikler = np.array([[
        veri["N"],
        veri["P"],
        veri["K"],
        veri["temperature"],
        veri["humidity"],
        veri["ph"],
        veri["rainfall"]
    ]])

    özellikler = scaler.transform(özellikler)
    tahmin = model.predict(özellikler)
    bitki = siniflar[tahmin[0]]

    return jsonify({"bitki": bitki})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001, debug=True)