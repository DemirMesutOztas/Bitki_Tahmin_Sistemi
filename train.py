import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.model_selection import train_test_split
import joblib

df = pd.read_csv("Crop_recommendation.csv")
print(df.head())
print(df.shape)
print(df["label"].value_counts())

X = df.drop("label", axis=1).values
y = df["label"].values

le = LabelEncoder()
y = le.fit_transform(y)

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

model = RandomForestClassifier(n_estimators=100, random_state=42)

model.fit(X_train, y_train)

y_pred = model.predict(X_test)

from sklearn.metrics import accuracy_score
print("Doğruluk:", accuracy_score(y_test, y_pred))
joblib.dump(model, "model.pkl")
joblib.dump(scaler, "scaler.pkl")

import json
siniflar = list(le.classes_)
with open("siniflar.json", "w") as f:
    json.dump(siniflar, f)
print("Kaydedildi!")