from flask import Flask, jsonify
from flask import request
from dbManager import DbManager

app = Flask(__name__)
db = DbManager()

@app.route('/api/departments/not_synced_records', methods=['GET'])
def get_deps_unsynced():
    unsynced_deps = db.get_unsynced_departments()
    return jsonify(unsynced_deps)

@app.route('/api/tasks_statuses/not_synced_records', methods=['GET'])
def get_tasks_statuses_unsynced():
    unsynced_tasks_statuses = db.get_unsynced_tasks_statuses()
    return jsonify(unsynced_tasks_statuses)

@app.route('/api/users/not_synced_records', methods=['GET'])
def get_users_unsynced():
    unsynced_users = db.get_unsynced_users()
    return jsonify(unsynced_users)

@app.route('/api/tasks/not_synced_records', methods=['GET'])
def get_task_unsynced():
    unsynced_tasks = db.get_unsynced_tasks()
    return jsonify(unsynced_tasks)

@app.route('/api/tasks', methods=['POST', 'PUT'])
def create_redact_task():
    task_record = request.json
    
    if request.method == 'POST':
        response = db.add_task(task_record)
        if response["status"] == 200:
            return jsonify(response), 200
        else:
            return jsonify(response), 400

    else:
        if task_record["update_time"] > db.get_sync_time("tasks", task_record["uuid"]):
            response = db.update_task(task_record)
            if response["status"] == 200:
                return jsonify(response), 200
            else:
                return jsonify(response), 400
        else:
            return jsonify({"status":409, "description":"the record won't be updated because the server's update record later"}), 409

@app.route('/api/tasks/delete/<string:uuid>/<string:client_update_time>', methods=['DELETE'])
def delete_task(uuid, client_update_time):
    if client_update_time > db.get_sync_time("tasks", uuid):
        response = db.delete_task(uuid)
        if response["status"] == 200:
            return jsonify(response), 200
        else:
            return jsonify(response), 400
    else:
        return jsonify({"status":409, "description":"the record won't be updated because the server's update record later"}), 409
    
@app.route('/api/successfullSync/<string:table_name>/<string:uuid>', methods=['PUT'])
def successfull_sync(table_name, uuid):
    response = db.update_record_sync(table_name, uuid)
    if response["status"] == 200:
        return jsonify(response), 200
    else:
        return jsonify(response), 400

@app.route('/api/successfullSyncDelete/<string:table_name>/<string:uuid>', methods=['DELETE'])
def successfull_sync_delete(table_name, uuid):
    response = db.delete_record(table_name, uuid)
    if response["status"] == 200:
        return jsonify(response), 200
    else:
        return jsonify(response), 400

if __name__ == '__main__':
    app.run(debug=True)
