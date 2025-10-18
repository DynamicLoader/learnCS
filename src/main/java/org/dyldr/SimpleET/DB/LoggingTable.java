package org.dyldr.SimpleET.DB;

import org.dyldr.SimpleET.Entity.LoggingEntity;
import org.springframework.data.repository.CrudRepository;

import java.util.Date;
import java.util.List;

public interface LoggingTable extends CrudRepository<LoggingEntity, String> {

    List<LoggingEntity> findAllByAction(LoggingEntity.ActionType action);

//    List<LoggingEntity> findAllByTimestampBetween(Date start, Date end);

    List<LoggingEntity> findAllByActionAndTimestampBetween(LoggingEntity.ActionType action, Date start, Date end);
}
