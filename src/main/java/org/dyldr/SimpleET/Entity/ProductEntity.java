package org.dyldr.SimpleET.Entity;

import lombok.Data;
import jakarta.persistence.*;

@Entity
@Data
public class ProductEntity {

        public enum Status {
                ON_SALE,
                OFF_SALE,
                DELETED
        }

        @Id
        @GeneratedValue(strategy=GenerationType.UUID)
        private String id;

        @Column(nullable = false)
        private String name;

        @Column()
        private String description;

        @Column(nullable = false)
        private Double price;

        @Column(nullable = false)
        private Integer stock;

        @Column(nullable = false)
        private String category;

        @Column()
        private String image;

        @ManyToOne
        @JoinColumn()
        private UserEntity seller;

        @Column(nullable = false)
        private double expressFee;

        @Column(nullable = false)
        private Integer totalSold = 0;

        @Column
        @Enumerated(EnumType.STRING)
        private Status status;
}
