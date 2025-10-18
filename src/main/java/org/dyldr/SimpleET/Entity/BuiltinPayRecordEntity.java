package org.dyldr.SimpleET.Entity;

import lombok.Data;
import jakarta.persistence.*;

import java.util.Date;

@Entity
@Data
public class BuiltinPayRecordEntity {
    @Id
    @GeneratedValue(strategy=GenerationType.UUID)
    private String id;

    @JoinColumn()
    @ManyToOne(fetch = FetchType.LAZY)
    private BuiltinPayAccountEntity account;

    @Column(nullable = false)
    private Double amount = 0.0;

    @Column(nullable = false)
    private String description;

    @Column(nullable = false)
    private Date createTime;

}
