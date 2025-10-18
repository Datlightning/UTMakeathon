from flask import Flask, render_template, request, redirect, url_for, flash, jsonify
import hashlib
# import firebase_admin
# from firebase_admin import credentials, firestore

# cred = credentials.Certificate("makeathon-d3ac7-firebase-adminsdk-fbsvc-f21fbe9467.json")
# firebase_admin.initialize_app(cred)
# db = firestore.client()  

app = Flask(__name__)
PASSWORD = "utmakeathon"
authenticated = False
app.secret_key = "supersecretkey"  # Needed for flash messages
DATA = {}
def hash_password(password: str) -> str:
    """Hashes a password using SHA-256."""
    sha = hashlib.sha256()
    sha.update(password.encode('utf-8'))
    return sha.hexdigest()
def get_data():
    with open("flask/database.txt", "r") as f:
        data = f.read()
        return eval(data)
def save_data(data):
    with open("flask/database.txt", "w") as f:
        f.write(str(data))  
        f.close()

@app.route('/')
def home():
    return render_template('index.html')
@app.route('/connect-storage', methods=['GET', 'POST'])
def connect_storage():
    global authenticated
    global PASSWORD
    if request.method == 'POST':
        password = request.form.get('password')

        # TODO: Add device authentication here
        if password == "password123":
            PASSWORD = hash_password(password)  # Store the password securely in a real app
            authenticated = True
            return redirect('/view-data')
        else:
            flash("Invalid username or password", "error")
            return redirect(url_for('connect_storage'))
    return render_template('connect_storage.html')
@app.route('/view-data', methods=['GET', 'POST'])
def view_data():
    if not authenticated:
        return render_template('accessdenied.html', redirect_url="/")
    if request.method == 'POST':
        # Handle container actions
        action = request.form.get('action')
        # container_type = request.form.get('type')
        # threshold = int(request.form.get('threshold'))
        if action  == "notifications":
            phone = request.form.get('phone')
            email = request.form.get('email')
            data = get_data()
            data[PASSWORD]['notifications']['phone'] = phone
            data[PASSWORD]['notifications']['email'] = email
            save_data(data)
            flash("Notification settings updated.", "success")
        elif action.startswith("confirm"):
            form_data = request.form.to_dict()
            container_id = request.form.get('container_id')
            data = get_data()
            idx = 0
            for container in data[PASSWORD]['containers']:
                if container['name'] == container_id:
                    container_id = idx
                    break
                idx += 1
            type = form_data['type']
            threshold = int(form_data['threshold'])
            container['type'] = type
            container['threshold'] = threshold
            data[PASSWORD]['containers'][idx] = container
            flash(f"Confirmed changes for {container['name']}.", "success")
            save_data(data)
        elif action.startswith("reset"):
            return render_template("dashboard.html", containers=containers, notifications=notifications)

    containers = get_data()[PASSWORD]['containers']
    notifications = get_data()[PASSWORD]['notifications']
    
    return render_template("dashboard.html", containers=containers, notifications=notifications)
@app.route('/handle-esp32-data', methods=['POST','GET'])
def handle_esp32_data():
    if request.method == 'POST':
        print(request)
        data = request.args.to_dict()
        print(data)
        if not data or 'password' not in data:
            return "Unauthorized", 401

        password = data.get('password')
        hashed_password = hash_password(password)
        db_data = get_data()
        if hashed_password not in db_data:
            return "Unauthorized", 401
        # Update container data sent by ESP32
        containers_data = data.get('containers', [])
        db_data[hashed_password]['containers'] = containers_data
        save_data(db_data)
        return "Data received", 200

    elif request.method == 'GET':
        # Provide container data to ESP32
        password = request.args.get('password')
        if not password:
            return "Unauthorized", 401
        hashed_password = hash_password(password)

        db_data = get_data()
        user_data = db_data.get(hashed_password, {'containers': [], 'notifications': {}})
        return jsonify(user_data['containers'])

    return "Invalid request", 400
@app.route('/signout')
def signout():
    global authenticated
    authenticated = False
    return redirect(url_for('home'))

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)