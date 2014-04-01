get '/' do
  haml :index
end

get '/leaderboard' do
  query = <<-SQL
      SELECT
        t.id AS team_id,
        t.name AS team_name, 
        SUM(c.points) AS score,
        MAX(s.created_at) AS last_scored_at
      FROM
        teams AS t
        INNER JOIN solutions AS s 
          ON s.team_id = t.id
        INNER JOIN challenges AS c 
          ON s.challenge_id = c.id
      GROUP BY t.id
      ORDER BY 
        score DESC,
        last_scored_at ASC,
        team_id ASC
    SQL

  content_type :json
  ActiveRecord::Base.connection.select_all(query).to_json
end

get '/solutions' do
  query = <<-SQL
    SELECT 
      challenges.name, 
      count(solutions.id) 
    FROM 
      challenges 
      RIGHT JOIN solutions 
        ON solutions.challenge_id = challenges.id 
    GROUP BY challenges.id
    ORDER BY
      challenges.name ASC
  SQL

  content_type :json
  ActiveRecord::Base.connection.select_all(query).to_json
end

get '/answers' do

  content_type :json
  'lmao as if'.to_json
end
