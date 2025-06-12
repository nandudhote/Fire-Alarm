from flask import Flask, request, jsonify

app = Flask(__name__)


@app.route('/alert_detected/<int:id>', methods=['GET'])
def alert_detected(id):
    print("Alert detected with ID:", id)
    print("Headers:", request.headers)
    print("Data:", request.data.decode('utf-8'))
    return jsonify({"status": "success", "id": id})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)