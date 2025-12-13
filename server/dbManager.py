import sqlite3

class DbManager:
    def _execute(self, query, params=(), commit_needed=False):
        try:
            con = self.con = sqlite3.connect("serverDb.db")
            print("successfull connected to db")
            con.row_factory = sqlite3.Row
            cursor = con.cursor()
            cursor.execute(query, params)

            if commit_needed:
                con.commit()
                return True
            
            return cursor.fetchall()
        except Exception as error:
            print(f"sql request error: {error}")

    def get_unsynced_tasks_statuses(self):
        results = self._execute("SELECT * FROM tasks_statuses WHERE sync_status=false")
        unsynced_tasks_statuses = [dict(row) for row in results]
        return unsynced_tasks_statuses

    def get_unsynced_departments(self):
        results = self._execute("SELECT * FROM departments WHERE sync_status=false")
        unsynced_deps = [dict(row) for row in results]
        return unsynced_deps

    def get_unsynced_users(self):
        results = self._execute("SELECT * FROM users WHERE sync_status=false")
        unsynced_users = [dict(row) for row in results]
        return unsynced_users

    def get_unsynced_tasks(self):
        results = self._execute("SELECT * FROM tasks WHERE sync_status=false")
        unsynced_tasks = [dict(row) for row in results]
        return unsynced_tasks

    def add_task(self, task_record):
        sql_request = """
            INSERT INTO tasks (
                uuid, sync_status, update_time, last_operation,
                assigner, assignee, create_date, due_date,
                fk_task_status_id, title, description, fk_department_id
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """

        record = (
            task_record["uuid"],
            1,
            task_record["update_time"],
            task_record["last_operation"],
            task_record["assigner"],
            task_record["assignee"],
            task_record["create_date"],
            task_record["due_date"],
            task_record["fk_task_status_id"],
            task_record["title"],
            task_record["description"],
            task_record["fk_department_id"]
        )
        if (self._execute(sql_request, record, commit_needed=True)):
            return {"status":200, "description":"record was successfuly inserted", "uuid": task_record["uuid"]}
        return {"status":400, "description":"error. check your request", "uuid": task_record["uuid"]}

    def update_task(self, task_record):
        query = """
            UPDATE tasks SET sync_status=?, update_time=?, 
                last_operation=?, assigner=?, assignee=?, create_date=?, 
                due_date=?, fk_task_status_id=?, title=?, description=?, 
                fk_department_id=? WHERE uuid=?
        """

        record = (
            1,
            task_record["update_time"],
            task_record["last_operation"],
            task_record["assigner"],
            task_record["assignee"],
            task_record["create_date"],
            task_record["due_date"],
            task_record["fk_task_status_id"],
            task_record["title"],
            task_record["description"],
            task_record["fk_department_id"],
            task_record["uuid"]
        )
        if (self._execute(query, record, True)):
            return {"status":200, "description":"record was successfuly updated", "uuid": task_record["uuid"]}
        return {"status":400, "description":"error. check your request", "uuid": task_record["uuid"]}

    def delete_task(self, uuid):
        if (self._execute("DELETE FROM tasks WHERE uuid=?", (uuid,), True)):
            return {"status":200, "description":"record was succsessfuly deleted", "uuid": uuid} 
        return {"status":400, "description":"error. can't find the record with uuid:{uuid}"}
    
    def get_sync_time(self, table_name, uuid):
        update_time = self._execute(f"SELECT update_time FROM {table_name} WHERE uuid=?", (uuid,))
        if(update_time):
            return update_time[0]["update_time"]
        return False

    def update_record_sync(self, table_name, uuid):
        query = f"UPDATE {table_name} SET sync_status=1 WHERE uuid=?"
        if (self._execute(query, (uuid,), True)):
            return {"status":200, "description":"record was succsessfuly synced"}
        return {"status":400, "description":"error. can't find the record"}

    def delete_record(self, table_name, uuid):
        query = f"DELETE FROM {table_name} WHERE uuid=?"
        if (self._execute(query, (uuid,), True)):
            return {"status":200, "description":"record was succsessfuly deleted"}
        return {"status":400, "description":"error. can't find the record"}