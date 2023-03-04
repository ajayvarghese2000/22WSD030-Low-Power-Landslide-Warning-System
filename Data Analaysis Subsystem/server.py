## A Flask app

from flask import Flask, request, jsonify
import json
from datetime import datetime

app = Flask(__name__)

@app.route('/rain', methods=['GET'])
def rain():
    # open the warnings.json file
    with open('warnings.json', 'r') as f:
        data = json.load(f)
        new_data = {"type": 0, "date": datetime.now().strftime("%d/%m/%Y %H:%M:%S")}
        data.append(new_data)
        # write the new data to the file
        with open('warnings.json', 'w') as f:
            json.dump(data, f)
            ## return Success HTTP status code
            return jsonify(new_data), 200
        
@app.route('/soil', methods=['GET'])
def soil():
    # open the warnings.json file
    with open('warnings.json', 'r') as f:
        data = json.load(f)
        new_data = {"type": 1, "date": datetime.now().strftime("%d/%m/%Y %H:%M:%S")}
        data.append(new_data)
        # write the new data to the file
        with open('warnings.json', 'w') as f:
            json.dump(data, f)
            ## return Success HTTP status code
            return jsonify(new_data), 200

@app.route('/seismic', methods=['GET'])
def seismic():
    # open the warnings.json file
    with open('warnings.json', 'r') as f:
        data = json.load(f)
        new_data = {"type": 2, "date": datetime.now().strftime("%d/%m/%Y %H:%M:%S")}
        data.append(new_data)
        # write the new data to the file
        with open('warnings.json', 'w') as f:
            json.dump(data, f)
            ## return Success HTTP status code
            return jsonify(new_data), 200
    
@app.route('/warnings', methods=['GET'])
def warnings():
    # open the warnings.json file
    with open('warnings.json', 'r') as f:
        data = json.load(f)
        return jsonify(data), 200
    
    return jsonify({"error": "Something went wrong"}), 500

if __name__ == "__main__":
    app.run(host='0.0.0.0', debug=True)